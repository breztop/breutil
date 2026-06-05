#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>

namespace bre {

struct TCPServerConfig {
    uint16_t port = 8080;

    int io_threads = std::thread::hardware_concurrency();
    int max_connections = 10000;
    std::chrono::seconds timeout{30};

    bool enable_keepalive = true;
    bool enable_nodelay = true;
    size_t buffer_size = 8192;

    constexpr bool Validate() const noexcept {
        return port > 0 && io_threads > 0 && max_connections > 0 && buffer_size > 0;
    }
};

struct TCPClientConfig {
    std::string host = "127.0.0.1";
    uint16_t port = 8080;

    std::chrono::seconds connect_timeout{10};
    std::chrono::seconds read_timeout{30};
    std::chrono::seconds write_timeout{30};

    bool enable_keepalive = true;
    bool enable_nodelay = true;
    int buffer_size = 8192;

    int reconnect_max_attempts = 3;
    std::chrono::milliseconds reconnect_delay{1000};

    constexpr bool Validate() const noexcept {
        return !host.empty() && port > 0 && buffer_size > 0 && reconnect_max_attempts >= 0;
    }
};

}  // namespace bre
