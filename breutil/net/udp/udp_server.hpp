#pragma once


#ifdef ASIO_STANDALONE
#include <asio.hpp>
using error_code = asio::error_code;
#else
#include <boost/asio.hpp>
namespace asio = boost::asio;
using error_code = boost::system::error_code;
#endif

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <span>
#include <string>
#include <vector>

#include "../asio_io_context_pool.hpp"
#include "breUtils/spdlog.hpp"

namespace bre {

using asio::ip::udp;

/**
 * @brief UDP服务端
 * 监听端口，接收数据，并可通过 SendTo 回复
 * 回调函数的数据不是你自己的
 */
class UDPServer {
public:
    using Shared = std::shared_ptr<UDPServer>;

    // the data is valid only within the callback
    using MessageCallback =
        std::function<void(const uint8_t* data, size_t size, const udp::endpoint&)>;

    static Shared Create(uint16_t port) { return std::make_shared<UDPServer>(port); }

    explicit UDPServer(uint16_t port)
        : _socket(AsioIOContextPool::Instance()->GetIOContext(), udp::endpoint(udp::v4(), port)) {}

    ~UDPServer() { Stop(); }

    void AddMessageCallback(MessageCallback callback, std::string host_regex,
                            std::string port_regex = ".*") {
        _callback_regexs.push_back({host_regex, port_regex, callback});
    }

    void Start() {
        do_receive();
        LOG_INFO("UDPServer started on port {}", _socket.local_endpoint().port());
    }

    void Stop() {
        error_code ec;
        _socket.close(ec);
    }

    // 回复消息给来源
    void SendTo(const std::vector<uint8_t>& data, const udp::endpoint& endpoint) {
        SendTo(std::make_shared<std::vector<uint8_t>>(data), endpoint);
    }

    void SendTo(const std::string& data, const udp::endpoint& endpoint) {
        SendTo(std::make_shared<std::vector<uint8_t>>(data.begin(), data.end()), endpoint);
    }

    void SendTo(std::span<const uint8_t> data, const udp::endpoint& endpoint) {
        auto buffer = std::make_shared<std::vector<uint8_t>>(data.begin(), data.end());
        SendTo(buffer, endpoint);
    }

    void SendTo(std::shared_ptr<std::vector<uint8_t>> data, const udp::endpoint& endpoint) {
        _socket.async_send_to(asio::buffer(*data), endpoint,
                              [data](error_code ec, std::size_t /*bytes_sent*/) {
                                  if (ec) {
                                      LOG_ERROR("UDPServer reply failed: {}", ec.message());
                                  }
                              });
    }

private:
    void do_receive() {
        _socket.async_receive_from(
            asio::buffer(_recv_buffer), _remote_endpoint,
            [this](error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    std::string key = _remote_endpoint.address().to_string();

                    for (const auto& cb_info : _callback_regexs) {
                        if (std::regex_match(key, std::regex(cb_info.host_regex)) &&
                            std::regex_match(std::to_string(_remote_endpoint.port()),
                                             std::regex(cb_info.port_regex))) {
                            cb_info.callback(_recv_buffer.data(), bytes_recvd, _remote_endpoint);
                        }
                    }

                    do_receive();
                } else if (ec != asio::error::operation_aborted) {
                    LOG_ERROR("UDP receive error: {}", ec.message());
                    if (_socket.is_open()) {
                        do_receive();
                    }
                }
            });
    }

private:
    struct callback_regex_info {
        std::string host_regex;
        std::string port_regex;
        MessageCallback callback;
    };

private:
    udp::socket _socket;
    udp::endpoint _remote_endpoint;  // 用于存储接收到的发送方地址

    std::array<uint8_t, 65536> _recv_buffer;

    std::vector<callback_regex_info> _callback_regexs;
};

}  // namespace bre
