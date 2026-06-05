#pragma once

#ifdef ASIO_STANDALONE
#include <asio.hpp>
using error_code = asio::error_code;
#else
#include <boost/asio.hpp>
namespace asio = boost::asio;
using error_code = boost::system::error_code;
#endif

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "../../spdlog.hpp"
#include "../asio_io_context_pool.hpp"
#include "tcp_config.hpp"
#include "tcp_session.hpp"

namespace bre {

using asio::ip::tcp;


class TCPClient : public std::enable_shared_from_this<TCPClient> {
public:
    using Shared = std::shared_ptr<TCPClient>;

    struct EventCallbacks {
        TCPSession::MessageCallback on_message = nullptr;

        std::function<void(TCPSession::Shared)> on_connected = nullptr;
        TCPSession::DisconnectCallback on_disconnect = nullptr;

        std::function<void(uint64_t, const std::string&)> on_failed = nullptr;
    };

    explicit TCPClient(const TCPClientConfig& config)
        : _config(config)
        , _io_context(AsioIOContextPool::Instance()->GetIOContext())
        , _connect_timer(_io_context) {
        if (!_config.Validate()) {
            throw std::invalid_argument("Invalid TCPClientConfig");
        }
    }

    ~TCPClient() {}

    TCPSession::Shared Connect(const EventCallbacks& callbacks) {
        auto session = TCPSession::Create(AsioIOContextPool::Instance()->GetIOContext());
        init_callbacks(session, callbacks);

        error_code ec;
        auto resolver = std::make_shared<tcp::resolver>(_io_context);
        auto endpoints = resolver->resolve(_config.host, std::to_string(_config.port), ec);
        if (ec) {
            handle_connect_error(callbacks, "Resolve failed: " + ec.message());
            return nullptr;
        }

        start_connect(session, endpoints, callbacks);
        return session;
    }

    /**
     * 异步连接服务器
     */
    void ConnectAync(const EventCallbacks& callbacks) {
        auto resolver = std::make_shared<tcp::resolver>(_io_context);

        resolver->async_resolve(
            _config.host, std::to_string(_config.port),
            [this, resolver, callbacks](error_code ec, tcp::resolver::results_type endpoints) {
                if (ec) {
                    handle_connect_error(callbacks, "Resolve failed: " + ec.message());
                    return;
                }
                start_connect_async(endpoints, callbacks);
            });
        return;
    }


private:
    void start_connect(TCPSession::Shared session, const tcp::resolver::results_type& endpoints,
                       const EventCallbacks& callbacks) {
        error_code ec;
        asio::connect(session->Socket(), endpoints, ec);
        if (ec) {
            handle_connect_error(callbacks, "Connect failed: " + ec.message());
            return;
        }

        session->SetInitSocket(true);
        session->Start();

        LOG_TRACE("Connected to {}:{}", _config.host, _config.port);
        if (callbacks.on_connected) {
            callbacks.on_connected(session);
        }
    }

    void start_connect_async(const tcp::resolver::results_type& endpoints,
                             const EventCallbacks& callbacks) {
        auto session = TCPSession::Create(AsioIOContextPool::Instance()->GetIOContext());
        init_callbacks(session, callbacks);

        // Connect timeout
        if (_config.connect_timeout.count() > 0) {
            _connect_timer.expires_after(_config.connect_timeout);
            _connect_timer.async_wait([session, callbacks](error_code ec) {
                if (!ec) {
                    // Timeout
                    if (session) {
                        session->Stop();  // This will cancel the connect operation
                    }
                    handle_connect_error(callbacks, "Connect timeout");
                }
            });
        }

        auto self = shared_from_this();
        asio::async_connect(
            session->Socket(), endpoints, [&, self](error_code ec, const tcp::endpoint&) {
                _connect_timer.cancel();

                if (ec) {
                    if (ec != asio::error::operation_aborted) {
                        handle_connect_error(callbacks, "Connect failed: " + ec.message());
                    }
                    return;
                }

                session->SetInitSocket(true);
                session->Start();

                LOG_DEBUG("Connected to {}:{}", _config.host, _config.port);
                if (callbacks.on_connected) {
                    callbacks.on_connected(session);
                } else {
                    LOG_WARN("No on_connected callback set");
                }
            });
    }


    void init_callbacks(TCPSession::Shared session, const EventCallbacks& callbacks) {
        // Set options
        session->SetOptions(_config.enable_keepalive, _config.enable_nodelay, _config.buffer_size);
        session->SetTimeouts(_config.read_timeout, _config.write_timeout);

        // Setup callbacks
        session->SetMessageCallback(callbacks.on_message);
        session->SetErrorCallback([callbacks](uint64_t sid, const std::string& msg) {
            if (callbacks.on_failed) {
                callbacks.on_failed(sid, msg);
            }
        });
        session->SetDisconnectCallback(callbacks.on_disconnect);
    }

    static void handle_connect_error(const EventCallbacks& callbacks, const std::string& msg) {
        LOG_ERROR("{}", msg);
        if (callbacks.on_failed) {
            callbacks.on_failed(0, msg);
        }
    }

private:
    TCPClientConfig _config;
    asio::io_context& _io_context;
    asio::steady_timer _connect_timer;
};

}  // namespace bre
