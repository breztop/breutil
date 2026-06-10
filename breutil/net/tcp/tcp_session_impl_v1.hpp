#pragma once

#ifdef ASIO_STANDALONE
#include <asio.hpp>
using error_code = asio::error_code;
#else
#include <boost/asio.hpp>
namespace asio = boost::asio;
using error_code = boost::system::error_code;
#endif

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include "../../block_queue.hpp"
#include "../net_header.hpp"

namespace bre::impl::v1 {

using asio::ip::tcp;


/**
 * - create session
 * - SetOptions; SetTimeouts
 * - Setup callbacks
 * - init socket
 * - start session
 *
 * session id for server
 */
class TCPSession : public std::enable_shared_from_this<TCPSession> {
public:
    using Shared = std::shared_ptr<TCPSession>;
    using MessageCallback =
        std::function<void(Shared, std::shared_ptr<std::vector<uint8_t>>)>;  // session, data
    using DisconnectCallback = std::function<void(uint64_t)>;                // session id
    using ErrorCallback = std::function<void(uint64_t, const std::string&)>;


    static Shared Create(asio::io_context& io_context) {
        return Shared(new TCPSession(io_context));
    }
    Shared SharedFromThis() { return shared_from_this(); }

    ~TCPSession() {
        Stop();
        // LOG_TRACE("Session {} destructed", _session_id);
    }

    TCPSession(const TCPSession&) = delete;
    TCPSession& operator=(const TCPSession&) = delete;

    tcp::socket& Socket() { return _socket; }
    void SetInitSocket(bool val) { _is_init_socket.store(val, std::memory_order_release); }
    uint64_t Id() const { return _session_id; }

    void SetMessageCallback(MessageCallback cb) { _message_callback = std::move(cb); }
    void SetDisconnectCallback(DisconnectCallback cb) { _disconnect_callback = std::move(cb); }
    void SetErrorCallback(ErrorCallback cb) { _error_callback = std::move(cb); }

    void SetOptions(bool keep_alive, bool no_delay, int buffer_size) {
        if (_is_closed) return;
        error_code ec;
        _socket.set_option(asio::socket_base::keep_alive(keep_alive), ec);
        _socket.set_option(tcp::no_delay(no_delay), ec);
        if (buffer_size > 0) {
            _socket.set_option(asio::socket_base::receive_buffer_size(buffer_size), ec);
            _socket.set_option(asio::socket_base::send_buffer_size(buffer_size), ec);
        }
    }

    void SetTimeouts(std::chrono::seconds read_timeout, std::chrono::seconds write_timeout) {
        _read_timeout = read_timeout;
        _write_timeout = write_timeout;
    }

    void Start() {
        if (!_is_init_socket.load(std::memory_order_acquire)) {
            std::cout << std::format("Session {} socket not initialized, cannot start",
                                     _session_id);
            return;
        }

        if (_is_closed) {
            return;
        }

        bool expected = false;
        if (!_is_started.compare_exchange_strong(expected, true)) {
            return;
        }

        reset_read_timer();
        async_read_header();
    }

    /**
     * once stopped, you need reinit socket to use it again
     */
    void Stop() {
        bool expected = false;
        if (!_is_closed.compare_exchange_strong(expected, true)) {
            // LOG_TRACE("Session {} already stopped", _session_id);
            return;
        }
        _is_init_socket = false;

        error_code ec;
        _read_timer.cancel();
        _write_timer.cancel();
        _socket.shutdown(tcp::socket::shutdown_both, ec);
        _socket.close(ec);

        if (_disconnect_callback) {
            _disconnect_callback(_session_id);
        }
    }

    void Send(std::span<const uint8_t> data) {
        if (_is_closed) {
            std::cout << std::format("Session {} is closed, you need keep the session live longer",
                                     _session_id);
            return;
        }

        auto self = shared_from_this();
        NetHeader header;

        auto header_data = header.GetHeaderData(data.size());
        auto buffer = std::make_shared<std::vector<uint8_t>>();
        buffer->reserve(header_data.size() + data.size());
        buffer->insert(buffer->end(), header_data.begin(), header_data.end());
        buffer->insert(buffer->end(), data.begin(), data.end());

        asio::post(_strand, [self, buffer]() {
            self->enqueue_write(buffer);
        });
    }

    void Send(const std::string& data) {
        Send(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.c_str()), data.size()));
    }

    void Send(const std::vector<uint8_t>& data) {
        Send(std::span<const uint8_t>(data.data(), data.size()));
    }

    void Send(std::shared_ptr<std::vector<uint8_t>> data) {
        Send(std::span<const uint8_t>(data->data(), data->size()));
    }

    bool IsClosed() const { return _is_closed; }


private:
    explicit TCPSession(asio::io_context& io_context)
        : _socket(io_context)
        , _strand(asio::make_strand(io_context))
        , _read_timer(io_context)
        , _write_timer(io_context)
        , _session_id(generate_session_id())
        , _is_closed(false)
        , _is_started(false) {}

    static uint64_t generate_session_id() {
        return _session_id_counter.fetch_add(1, std::memory_order_relaxed);
    }

    void reset_read_timer() {
        if (_read_timeout.count() == 0) return;

        _read_timer.expires_after(_read_timeout);
        auto self = shared_from_this();
        _read_timer.async_wait([this, self](error_code ec) {
            if (!ec) {
                handle_error("Read timeout");
                Stop();
            }
        });
    }

    void async_read_header() {
        auto self = shared_from_this();
        asio::async_read(_socket, asio::buffer(_header_data, NetHeader::HeaderSize),
                         [this, self](error_code ec, std::size_t /*length*/) {
                             if (ec) {
                                 if (ec != asio::error::operation_aborted) {
                                     if (ec != asio::error::eof) {
                                         handle_error("Read header failed: " + ec.message());
                                     }
                                     Stop();
                                 }
                                 return;
                             }

                             reset_read_timer();

                             if (!_header_reader.Parse(std::vector<uint8_t>(
                                     _header_data, _header_data + NetHeader::HeaderSize))) {
                                 handle_error("Parse header failed");
                                 Stop();
                                 return;
                             }

                             async_read_data(_header_reader.FileSize);
                         });
    }

    void async_read_data(size_t length) {
        if (length == 0) {
            async_read_header();
            return;
        }

        auto self = shared_from_this();
        auto buffer = std::make_shared<std::vector<uint8_t>>(length);

        asio::async_read(_socket, asio::buffer(*buffer), asio::transfer_exactly(length),
                         [this, self, buffer](error_code ec, std::size_t /*length*/) {
                             if (ec) {
                                 if (ec != asio::error::operation_aborted) {
                                     handle_error("Read data failed: " + ec.message());
                                     Stop();
                                 }
                                 return;
                             }

                             reset_read_timer();

                             if (_message_callback) {
                                 _message_callback(self, buffer);
                             }

                             async_read_header();
                         });
    }

    void enqueue_write(std::shared_ptr<std::vector<uint8_t>> data) {
        _write_queue.push_back(data);
        if (_is_writing) {
            return;
        }
        _is_writing = true;
        do_write();
    }

    void do_write() {
        if (_write_queue.empty()) {
            _is_writing = false;
            return;
        }

        auto self = shared_from_this();
        const auto& payload = _write_queue.front();

        if (_write_timeout.count() > 0) {
            _write_timer.expires_after(_write_timeout);
            _write_timer.async_wait([this, self](error_code ec) {
                if (!ec) {
                    handle_error("Write timeout");
                    Stop();
                }
            });
        }

        asio::async_write(_socket, asio::buffer(*payload),
                          asio::bind_executor(_strand, [this, self, payload](
                                                           error_code ec, std::size_t /*length*/) {
                              // Cancel write timer
                              _write_timer.cancel();

                              if (ec) {
                                  if (ec != asio::error::operation_aborted) {
                                      handle_error("Write failed: " + ec.message());
                                      _is_writing = false;
                                      Stop();
                                  }
                                  return;
                              }

                              _write_queue.pop_front();
                              do_write();
                          }));
    }

    void handle_error(const std::string& error_msg) {
        if (_error_callback) {
            _error_callback(_session_id, error_msg);
        } else {
            std::cout << std::format("Session {} error: {}", _session_id, error_msg);
        }
    }

private:
    tcp::socket _socket;
    asio::strand<tcp::socket::executor_type> _strand;
    asio::steady_timer _read_timer;
    asio::steady_timer _write_timer;

    uint64_t _session_id;

    std::atomic<bool> _is_init_socket{false};
    std::atomic<bool> _is_closed;
    std::atomic<bool> _is_started;
    bool _is_writing{false};

    NetHeader _header_reader;
    char _header_data[NetHeader::HeaderSize];

    MessageCallback _message_callback;
    DisconnectCallback _disconnect_callback;
    ErrorCallback _error_callback;

    std::deque<std::shared_ptr<std::vector<uint8_t>>> _write_queue;

    std::chrono::seconds _read_timeout{30};
    std::chrono::seconds _write_timeout{30};

    inline static std::atomic<uint64_t> _session_id_counter{1};
};

}  // namespace bre::impl::v1
