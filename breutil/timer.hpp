#pragma once

/** timer.hpp
 * CountTimer: 简单计时器，对chrono进行了封装
 * ProgressTimer: 进度计时器
 * Timer: 单一任务定时器，支持同步和异步
 * MultiTaskTimer: 多任务定时器， 基于优先队列实现
 */

#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>


namespace bre {

// timer
// 简单计时器，对chrono进行了封装
class CountTimer {
public:
    CountTimer() { Restart(); }

    void Restart() { _start_time = std::chrono::high_resolution_clock::now(); }

    double Elapsed() const {
        using namespace std::chrono;
        return duration<double>(high_resolution_clock::now() - _start_time).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start_time;
};  // timer

// 进度计时器
class ProgressTimer {
public:
    ProgressTimer() { _timer.Restart(); }

    ~ProgressTimer() { std::cout << "Time elapsed: " << _timer.Elapsed() << 's' << std::endl; }

private:
    CountTimer _timer;
};

// 单一任务定时器，支持同步和异步
class Timer {
public:
    using Shared = std::shared_ptr<Timer>;
    Timer(int inertval_ms = 0) {
        m_Interval = inertval_ms;
        m_Stop = true;
    }

    ~Timer() {
        Cancel();
        if (m_Thread.joinable()) {
            m_Thread.join();
        }
    }

    void Wait(std::function<void()> func) {
        using namespace std::chrono;
        std::this_thread::sleep_for(milliseconds(m_Interval));
        func();
        m_Stop = true;
    }

    void AsyncWait(std::function<void()> func, bool is_loop = false) {
        using namespace std::chrono;

        m_Stop = false;
        m_Thread = std::thread([this, func, is_loop]() {
            while (!m_Stop) {
                auto next_wake = steady_clock::now() + milliseconds(m_Interval);
                {
                    std::unique_lock<std::mutex> lock(m_ThreadMutex);
                    if (m_ExpiredConditionVar.wait_until(lock, next_wake, [this] {
                            return m_Stop.load();
                        })) {
                        break;
                    }
                }
                func();
                if (!is_loop) {
                    break;
                }
            }
        });
    }
    void Cancel() {
        std::lock_guard<std::mutex> lock(m_ThreadMutex);
        m_Stop = true;
        m_ExpiredConditionVar.notify_one();
    }

private:
    std::atomic_bool m_Stop;  // 是否到期

    std::mutex m_ThreadMutex;
    std::condition_variable m_ExpiredConditionVar;

    unsigned int m_Interval;  // 超时时间 ms
    std::thread m_Thread;
};


// 多任务定时器， 基于优先队列实现
class MultiTaskTimer {
    struct TimerTask {
        std::chrono::steady_clock::time_point due_time;
        std::function<void()> callback;

        bool operator>(const TimerTask& other) const { return due_time > other.due_time; }
    };

public:
    int Add(int delay, std::function<void()> callback) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto due_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);
        _tasks.emplace(TimerTask{due_time, callback});
        _cv.notify_one();
        return _tasks.size();
    }

    void Run() {
        while (true) {
            TimerTask task;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (!_tasks.empty()) {
                    auto now = std::chrono::steady_clock::now();
                    if (_tasks.top().due_time <= now) {
                        task = _tasks.top();
                        _tasks.pop();
                    } else {
                        _cv.wait_until(lock, _tasks.top().due_time);
                        continue;
                    }
                } else {
                    _cv.wait(lock);
                    continue;
                }
            }

            if (task.callback) {
                task.callback();
            }

            if (_tasks.empty()) {
                break;
            }
        }
    }

private:
    std::priority_queue<TimerTask, std::vector<TimerTask>, std::greater<>> _tasks;
    std::mutex _mutex;
    std::condition_variable _cv;
};

}  // namespace bre