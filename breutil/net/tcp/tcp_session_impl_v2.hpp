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

namespace bre::impl::v2 {

using asio::ip::tcp;
using CoroReadHandler = asio::awaitable<void, asio::any_io_executor>;


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

        // 启动读写协程
        auto self = shared_from_this();
        asio::co_spawn(
            _strand,
            [this, self]() {
                return coro_read_loop();
            },
            asio::detached);
        asio::co_spawn(
            _strand,
            [this, self]() {
                return coro_write_loop();
            },
            asio::detached);
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

        NetHeader header;
        auto header_data = header.GetHeaderData(data.size());
        auto buffer = std::make_shared<std::vector<uint8_t>>();
        buffer->reserve(header_data.size() + data.size());
        buffer->insert(buffer->end(), header_data.begin(), header_data.end());
        buffer->insert(buffer->end(), data.begin(), data.end());

        asio::post(_strand, [this, buffer]() {
            _write_queue.push_back(buffer);
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
        , _session_id(generate_session_id())
        , _is_closed(false)
        , _is_started(false) {}

    static uint64_t generate_session_id() {
        return _session_id_counter.fetch_add(1, std::memory_order_relaxed);
    }

    // 读循环协程：处理头部和数据的读取
    asio::awaitable<void, asio::any_io_executor> coro_read_loop() {
        auto executor = co_await asio::this_coro::executor;
        auto self = shared_from_this();

        while (!_is_closed) {
            try {
                // 读取头部
                std::vector<uint8_t> header_buf(NetHeader::HeaderSize);
                [[maybe_unused]] auto bytes_read = co_await asio::async_read(
                    _socket, asio::buffer(header_buf), asio::use_awaitable);

                if (_is_closed) break;

                // 解析头部
                if (!_header_reader.Parse(header_buf)) {
                    handle_error("Parse header failed");
                    Stop();
                    break;
                }

                uint32_t data_size = _header_reader.FileSize;

                // 读取数据
                if (data_size > 0) {
                    auto data_buffer = std::make_shared<std::vector<uint8_t>>(data_size);

                    // 设置读超时
                    if (_read_timeout.count() > 0) {
                        asio::steady_timer read_timer(executor);
                        read_timer.expires_after(_read_timeout);

                        auto [read_ec, read_bytes] = co_await asio::async_read(
                            _socket, asio::buffer(*data_buffer), asio::transfer_exactly(data_size),
                            asio::as_tuple(asio::use_awaitable));

                        read_timer.cancel();

                        if (read_ec && read_ec != asio::error::operation_aborted) {
                            handle_error("Read data failed: " + read_ec.message());
                            Stop();
                            break;
                        }
                    } else {
                        auto [read_ec, read_bytes] = co_await asio::async_read(
                            _socket, asio::buffer(*data_buffer), asio::transfer_exactly(data_size),
                            asio::as_tuple(asio::use_awaitable));

                        if (read_ec && read_ec != asio::error::operation_aborted) {
                            handle_error("Read data failed: " + read_ec.message());
                            Stop();
                            break;
                        }
                    }

                    if (!_is_closed && _message_callback) {
                        _message_callback(self, data_buffer);
                    }
                }
            } catch (const std::exception& e) {
                if (!_is_closed) {
                    handle_error(std::string("Read exception: ") + e.what());
                    Stop();
                }
                break;
            }
        }

        // LOG_TRACE("Session {} read loop ended", _session_id);
    }

    // 写循环协程：处理队列中的数据发送
    asio::awaitable<void, asio::any_io_executor> coro_write_loop() {
        auto executor = co_await asio::this_coro::executor;

        while (!_is_closed) {
            try {
                // 等待一小段时间或直到队列非空
                std::shared_ptr<std::vector<uint8_t>> write_data;

                bool has_data = false;
                for (int retry = 0; retry < 10 && !has_data && !_is_closed; ++retry) {
                    if (!_write_queue.empty()) {
                        write_data = _write_queue.front();
                        _write_queue.pop_front();
                        has_data = true;
                        break;
                    }

                    // 短暂延迟再检查
                    asio::steady_timer wait_timer(executor);
                    wait_timer.expires_after(std::chrono::milliseconds(10));
                    [[maybe_unused]] auto [ec] =
                        co_await wait_timer.async_wait(asio::as_tuple(asio::use_awaitable));
                }

                if (!write_data || _is_closed) {
                    continue;
                }

                // 写数据，带超时
                if (_write_timeout.count() > 0) {
                    asio::steady_timer write_timer(executor);
                    write_timer.expires_after(_write_timeout);

                    auto [write_ec, write_bytes] = co_await asio::async_write(
                        _socket, asio::buffer(*write_data), asio::as_tuple(asio::use_awaitable));

                    write_timer.cancel();

                    if (write_ec && write_ec != asio::error::operation_aborted) {
                        handle_error("Write failed: " + write_ec.message());
                        Stop();
                        break;
                    }
                } else {
                    auto [write_ec, write_bytes] = co_await asio::async_write(
                        _socket, asio::buffer(*write_data), asio::as_tuple(asio::use_awaitable));

                    if (write_ec && write_ec != asio::error::operation_aborted) {
                        handle_error("Write failed: " + write_ec.message());
                        Stop();
                        break;
                    }
                }
            } catch (const std::exception& e) {
                if (!_is_closed) {
                    handle_error(std::string("Write exception: ") + e.what());
                    Stop();
                }
                break;
            }
        }

        // LOG_TRACE("Session {} write loop ended", _session_id);
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

    uint64_t _session_id;

    std::atomic<bool> _is_init_socket{false};
    std::atomic<bool> _is_closed;
    std::atomic<bool> _is_started;

    NetHeader _header_reader;

    MessageCallback _message_callback;
    DisconnectCallback _disconnect_callback;
    ErrorCallback _error_callback;

    std::deque<std::shared_ptr<std::vector<uint8_t>>> _write_queue;

    std::chrono::seconds _read_timeout{30};
    std::chrono::seconds _write_timeout{30};

    inline static std::atomic<uint64_t> _session_id_counter{1};
};

}  // namespace bre::impl::v2
