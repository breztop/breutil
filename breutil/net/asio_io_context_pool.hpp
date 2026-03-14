#pragma once

#include "../singleton.hpp"

#ifdef ASIO_STANDALONE
#include <asio.hpp>
using namespace asio;
#else
#include <boost/asio.hpp>
namespace asio = boost::asio;
#endif

#include <iostream>
#include <thread>
#include <vector>

namespace bre {

class AsioIOContextPool : public Singleton<AsioIOContextPool> {
    friend Singleton<AsioIOContextPool>;

public:
    using IOContext = asio::io_context;
    using Work = asio::executor_work_guard<asio::io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;

    // 使用 round-robin 的方式返回一个 io_context --> 负载均衡
    asio::io_context& GetIOContext();

    void Stop();


    ~AsioIOContextPool();
    AsioIOContextPool(const AsioIOContextPool&) = delete;
    AsioIOContextPool& operator=(const AsioIOContextPool&) = delete;

private:
    AsioIOContextPool(std::size_t size = std::thread::hardware_concurrency());

private:
    int _pool_size;
    std::vector<IOContext> _io_contexts;
    std::vector<WorkPtr> _works;   // 保持 io_context 的工作
    std::vector<std::thread> _threads;
    std::size_t _next_io_context;  // 下一个 io_context 的索引
};

// implementation

inline AsioIOContextPool::AsioIOContextPool(std::size_t size)
    : _pool_size(size < 2 ? 2 : size)
    , _io_contexts(_pool_size)
    , _works(_pool_size)
    , _next_io_context(0) {
    for (std::size_t i = 0; i < size; ++i) {
        _works[i] = std::make_unique<Work>(asio::make_work_guard(_io_contexts[i]));
    }

    for (std::size_t i = 0; i < _io_contexts.size(); ++i) {
        try {
            _threads.emplace_back([this, i]() {
                _io_contexts[i].run();
            });
        } catch (const std::exception& e) {
            std::cerr << "Error starting thread: " << e.what() << '\n';
        }
    }
}

inline AsioIOContextPool::~AsioIOContextPool() {
    Stop();
    std::cout << "AsioIOContextPool destruct\n";
}

inline asio::io_context& AsioIOContextPool::GetIOContext() {
    auto& context = _io_contexts[_next_io_context++ % _io_contexts.size()];
    // 多线程抢占无所谓，主要是用于分配
    if (_next_io_context >= _io_contexts.size()) {
        _next_io_context = 0;
    }
    return context;
}

inline void AsioIOContextPool::Stop() {
    // 当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
    for (auto& context : _io_contexts) {
        // 把服务先停止
        context.stop();
    }

    for (auto& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    for (auto& work : _works) {
        work.reset();
    }
}

}  // namespace bre
