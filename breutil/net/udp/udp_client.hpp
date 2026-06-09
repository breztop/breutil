#pragma once


#ifdef ASIO_STANDALONE
#include <asio.hpp>
using error_code = asio::error_code;
#else
#include <boost/asio.hpp>
namespace asio = boost::asio;
using error_code = boost::system::error_code;
#endif

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "../../spdlog.hpp"
#include "../asio_io_context_pool.hpp"

namespace bre {

using asio::ip::udp;

/**
 * @brief UDP客户端，用于发送数据
 * 每次发送数据大小 不超过 64KB， 最佳数据大小 1452 字节以内
 */
class UDPClient {
public:
    using Shared = std::shared_ptr<UDPClient>;

    static Shared Create(const std::string& host, uint16_t port) {
        return std::make_shared<UDPClient>(host, port);
    }

    UDPClient(const std::string& host, uint16_t port)
        : _socket(AsioIOContextPool::Instance()->GetIOContext()) {
        _socket.open(udp::v4());

        error_code ec;
        udp::resolver resolver(AsioIOContextPool::Instance()->GetIOContext());
        auto endpoints = resolver.resolve(udp::v4(), host, std::to_string(port), ec);
        if (!ec && endpoints.begin() != endpoints.end()) {
            _remote_endpoint = *endpoints.begin();
        } else {
            LOG_ERROR("UDP resolve failed: {}", ec.message());
        }
    }

    ~UDPClient() {
        error_code ec;
        _socket.close(ec);
    }

    void Send(const std::string& data) { Send(std::vector<uint8_t>(data.begin(), data.end())); }

    void Send(const std::vector<uint8_t>& data) {
        Send(std::make_shared<std::vector<uint8_t>>(data));
    }

    void Send(std::span<const uint8_t> data) {
        Send(std::make_shared<std::vector<uint8_t>>(data.begin(), data.end()));
    }

    void Send(std::shared_ptr<std::vector<uint8_t>> data) {
        _socket.async_send_to(asio::buffer(*data), _remote_endpoint,
                              [data](error_code ec, std::size_t /*bytes_sent*/) {
                                  if (ec) {
                                      LOG_ERROR("UDP send failed: {}", ec.message());
                                  }
                              });
    }

private:
    udp::socket _socket;
    udp::endpoint _remote_endpoint;
};


}  // namespace bre
