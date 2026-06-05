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
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>

#include "../../spdlog.hpp"
#include "../asio_io_context_pool.hpp"
#include "tcp_config.hpp"
#include "tcp_session.hpp"

namespace bre {

using asio::ip::tcp;

// 前置声明
class TCPServer;

class TCPServer : public std::enable_shared_from_this<TCPServer> {
public:
    using Shared = std::shared_ptr<TCPServer>;
    using SessionPtr = TCPSession::Shared;

    struct EventCallbacks {
        TCPSession::MessageCallback on_message = nullptr;
        std::function<void(SessionPtr)> on_connected = nullptr;
        TCPSession::DisconnectCallback on_disconnect = nullptr;
        TCPSession::ErrorCallback on_error = nullptr;
    };

    static Shared Create(const TCPServerConfig& config, const EventCallbacks& callbacks) {
        return std::make_shared<TCPServer>(config, callbacks);
    }

    TCPServer(const TCPServerConfig& config, const EventCallbacks& callbacks)
        : _config(config)
        , _callbacks(callbacks)
        , _io_context()
        , _acceptor(_io_context, tcp::endpoint(tcp::v4(), config.port))
        , _is_running(false) {
        error_code ec;
        _acceptor.set_option(tcp::acceptor::reuse_address(true), ec);
        if (ec) {
            LOG_ERROR("Failed to set reuse_address: {}", ec.message());
        }
    }

    ~TCPServer() {
        Stop();
        LOG_INFO("TCPServer on port {} stopped", _config.port);
    }

    TCPServer(const TCPServer&) = delete;
    TCPServer& operator=(const TCPServer&) = delete;

    void Start() {
        if (_is_running) {
            LOG_WARN("Server already running");
            return;
        }

        _is_running = true;
        start_accept();
        LOG_INFO("TCPServer started on port {}", _config.port);
    }

    // 阻塞运行
    void Run() {
        if (!_is_running) {
            Start();
        }
        _io_context.run();
    }

    void Stop() {
        if (!_is_running) {
            return;
        }

        _is_running = false;

        // 关闭acceptor
        error_code ec;
        _acceptor.close(ec);

        std::vector<uint64_t> session_ids;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (const auto& [id, _] : _sessions) {
                session_ids.push_back(id);
            }
        }

        for (auto id : session_ids) {
            DisconnectSession(id);
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _sessions.clear();
        }

        _io_context.stop();
    }

    // 发送消息到指定会话
    void Send(uint64_t session_id, std::span<const uint8_t> data) {
        auto session = GetSession(session_id);
        if (session) {
            session->Send(data);
        } else {
            LOG_WARN("Session {} not found", session_id);
        }
    }

    // 广播消息到所有会话
    void Broadcast(std::span<const uint8_t> data) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& [id, session] : _sessions) {
            if (session) {
                session->Send(data);
            }
        }
    }

    // 获取会话
    SessionPtr GetSession(uint64_t session_id) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(session_id);
        return (it != _sessions.end()) ? it->second : nullptr;
    }

    // 获取所有会话ID
    std::vector<uint64_t> GetSessionIds() {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<uint64_t> ids;
        ids.reserve(_sessions.size());
        for (const auto& [id, _] : _sessions) {
            ids.push_back(id);
        }
        return ids;
    }

    // 获取会话数量
    size_t GetSessionCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _sessions.size();
    }

    // 断开指定会话
    void DisconnectSession(uint64_t session_id) {
        auto session = GetSession(session_id);
        if (session) {
            session->Stop();
        }
    }


    // 获取IO上下文
    asio::io_context& GetIOContext() { return _io_context; }

    // 获取服务器配置
    const TCPServerConfig& GetConfig() const { return _config; }

private:
    void start_accept() {
        auto session = TCPSession::Create(AsioIOContextPool::Instance()->GetIOContext());

        // 设置会话配置
        session->SetOptions(_config.enable_keepalive, _config.enable_nodelay, _config.buffer_size);
        session->SetTimeouts(std::chrono::seconds(_config.timeout.count()),
                             std::chrono::seconds(_config.timeout.count()));

        // 设置会话回调
        setup_session_callbacks(session);

        auto self = shared_from_this();
        _acceptor.async_accept(session->Socket(), [this, session, self](error_code ec) {
            handle_accept(session, ec);
        });
    }

    void handle_accept(SessionPtr session, error_code ec) {
        if (!ec) {
            // 获取客户端信息
            error_code info_ec;
            auto remote_endpoint = session->Socket().remote_endpoint(info_ec);
            std::string client_info;
            if (!info_ec) {
                client_info = remote_endpoint.address().to_string() + ":" +
                              std::to_string(remote_endpoint.port());
            } else {
                client_info = "unknown";
            }

            // 添加到会话列表
            uint64_t session_id = session->Id();
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _sessions[session_id] = session;
            }

            // 标记socket已初始化
            session->SetInitSocket(true);

            // 启动会话
            session->Start();

            // 调用连接回调
            if (_callbacks.on_connected) {
                _callbacks.on_connected(session);
            }

            LOG_TRACE("Client connected: {} ({}), total sessions: {}", session_id, client_info,
                      GetSessionCount());
        } else {
            if (ec != asio::error::operation_aborted) {
                LOG_ERROR("Accept failed: {}", ec.message());
            }
        }

        // 继续接受新连接
        if (_is_running) {
            start_accept();
        }
    }

    void setup_session_callbacks(SessionPtr session) {
        // 消息回调
        session->SetMessageCallback(_callbacks.on_message);

        // 断开连接回调
        session->SetDisconnectCallback([this](uint64_t sid) {
            {
                std::lock_guard<std::mutex> lock(_mutex);
                auto it = _sessions.find(sid);
                if (it != _sessions.end()) {
                    _sessions.erase(it);

                    if (_callbacks.on_disconnect) {
                        _callbacks.on_disconnect(sid);
                    }
                }
            }

            LOG_TRACE("Client disconnected: {}, remaining sessions: {}", sid, GetSessionCount());
        });

        // 错误回调
        session->SetErrorCallback([this](uint64_t sid, const std::string& error) {
            if (_callbacks.on_error) {
                _callbacks.on_error(sid, error);
            }
            LOG_DEBUG("Session {} error: {}", sid, error);
        });
    }

    size_t SessionCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _sessions.size();
    }

private:
    TCPServerConfig _config;
    EventCallbacks _callbacks;
    asio::io_context _io_context;
    tcp::acceptor _acceptor;
    std::atomic<bool> _is_running;

    std::map<uint64_t, SessionPtr> _sessions;  // ID -> Session
    mutable std::mutex _mutex;
};

}  // namespace bre
