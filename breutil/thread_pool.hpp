#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>


namespace bre {

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
    ~ThreadPool();

    template <typename F, typename... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    /**
     * @brief 获取线程池中的线程数量
     */
    size_t GetThreadCount() const;

    /**
     * @brief 获取等待队列中的任务数量
     */
    size_t GetQueueSize() const;

    /**
     * @brief 获取活跃线程数量
     */
    size_t GetActiveThreadCount() const;

    /**
     * @brief 等待所有任务完成
     */
    void WaitAll();

private:
    std::vector<std::thread> _workers;

    std::atomic<size_t> _activeThreads; // 活跃线程计数
    std::atomic_bool _stop;

    std::queue<std::function<void()>> _tasks;
    mutable std::mutex _queueMutex;
    std::condition_variable_any _condVar;
};

// implementation

inline size_t ThreadPool::GetThreadCount() const {
    return _workers.size();
}

inline size_t ThreadPool::GetQueueSize() const {
    std::unique_lock<std::mutex> lock(_queueMutex);
    return _tasks.size();
}

inline size_t ThreadPool::GetActiveThreadCount() const {
    return _activeThreads.load(std::memory_order_relaxed);
}

inline void ThreadPool::WaitAll() {
    std::unique_lock<std::mutex> lock(_queueMutex);
    _condVar.wait(lock, [this] {
        return _tasks.empty() && _activeThreads.load(std::memory_order_relaxed) == 0;
    });
}

inline ThreadPool::~ThreadPool() {
    {
        std::lock_guard lock(_queueMutex);
        _stop = true;
    }
    _condVar.notify_all();
    for (auto& worker : _workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

inline ThreadPool::ThreadPool(size_t threadCount) {
    _stop = false;
    if (threadCount == 0) {
        threadCount = 2;
    }

    for (size_t i = 0; i < threadCount; ++i) {
        _workers.emplace_back([this]() {
            while (!_stop) {
                std::function<void()> task;
                {
                    std::unique_lock lock(_queueMutex);
                    _condVar.wait(lock, [this] {
                        return _stop || !_tasks.empty();
                    });
                    if (_stop && _tasks.empty()) return;
                    if (!_tasks.empty()) {
                        task = std::move(_tasks.front());
                        _tasks.pop();
                    } else {
                        continue;
                    }
                }
                _activeThreads.fetch_add(1, std::memory_order_relaxed);
                task();
                _activeThreads.fetch_sub(1, std::memory_order_relaxed);
            }
        });
    }
}


template <typename F, typename... Args>
inline auto ThreadPool::Enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;
    auto taskPtr = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    {
        std::lock_guard lock(_queueMutex);
        if (_stop) {
            return std::future<return_type>();  // Return an empty future if the pool is stopped
        }
        _tasks.emplace([taskPtr]() {
            (*taskPtr)();
        });
    }
    _condVar.notify_one();
    return taskPtr->get_future();
}



}  // namespace bre