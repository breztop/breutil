#pragma once

#include <pthread.h>
#include <signal.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <future>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bre::os {

// 常用信号常量（与 POSIX 定义一致）
enum class Signal {
    INT = SIGINT,
    TERM = SIGTERM,
    KILL = SIGKILL,
    HUP = SIGHUP,
    QUIT = SIGQUIT,
    USR1 = SIGUSR1,
    USR2 = SIGUSR2,
    PIPE = SIGPIPE,
    ALRM = SIGALRM,
    CHLD = SIGCHLD,
};

namespace signal {

using SignalHandler = std::function<void(Signal)>;

// 内部管理类
class SignalManager {
public:
    using HandlerId = int;

    SignalManager() : m_nextId(1), m_stopFlag(false), m_running(false) {
        // 初始化唤醒信号（使用 SIGRTMIN，确保不被用户占用）
        m_wakeupSig = SIGRTMIN;
    }

    ~SignalManager() { stopAllInternal(); }

    // 注册监听
    HandlerId notify(SignalHandler handler, const std::vector<Signal>& signals) {
        if (!handler) {
            throw std::runtime_error("signal: nil handler");
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        HandlerId id = m_nextId++;
        HandlerInfo info{id, std::move(handler), signals};

        m_handlers[id] = info;
        for (auto sig : signals) {
            m_sigToHandlers[static_cast<int>(sig)].insert(id);
        }

        // 更新全局信号掩码并重启线程
        reconfigureLocked();
        return id;
    }

    // 停止监听
    void stop(HandlerId id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_handlers.find(id);
        if (it == m_handlers.end()) return;

        // 从映射中移除
        for (auto sig : it->second.signals) {
            int sigNum = static_cast<int>(sig);
            auto& handlers = m_sigToHandlers[sigNum];
            handlers.erase(id);
            if (handlers.empty()) m_sigToHandlers.erase(sigNum);
        }
        m_handlers.erase(it);

        reconfigureLocked();
    }

    void stopAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_handlers.clear();
        m_sigToHandlers.clear();
        reconfigureLocked();
    }

    // 重置信号处理为默认
    void reset(const std::vector<Signal>& signals) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto sig : signals) {
            int sigNum = static_cast<int>(sig);
            // 从所有 handler 中移除该信号
            auto it = m_sigToHandlers.find(sigNum);
            if (it != m_sigToHandlers.end()) {
                for (auto hid : it->second) {
                    auto& handlerSignals = m_handlers[hid].signals;
                    handlerSignals.erase(
                        std::remove(handlerSignals.begin(), handlerSignals.end(), sig),
                        handlerSignals.end());
                }
                m_sigToHandlers.erase(it);
            }
            // 设置默认处理
            struct sigaction sa;
            memset(&sa, 0, sizeof(sa));
            sa.sa_handler = SIG_DFL;
            sigaction(sigNum, &sa, nullptr);
            // 从阻塞集中移除
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, sigNum);
            pthread_sigmask(SIG_UNBLOCK, &set, nullptr);
        }
        reconfigureLocked();
    }

    // 忽略信号
    void ignore(const std::vector<Signal>& signals) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto sig : signals) {
            int sigNum = static_cast<int>(sig);
            // 从所有 handler 中移除该信号
            auto it = m_sigToHandlers.find(sigNum);
            if (it != m_sigToHandlers.end()) {
                for (auto hid : it->second) {
                    auto& handlerSignals = m_handlers[hid].signals;
                    handlerSignals.erase(
                        std::remove(handlerSignals.begin(), handlerSignals.end(), sig),
                        handlerSignals.end());
                }
                m_sigToHandlers.erase(it);
            }
            // 设置忽略
            struct sigaction sa;
            memset(&sa, 0, sizeof(sa));
            sa.sa_handler = SIG_IGN;
            sigaction(sigNum, &sa, nullptr);
            // 从阻塞集中移除
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, sigNum);
            pthread_sigmask(SIG_UNBLOCK, &set, nullptr);
        }
        reconfigureLocked();
    }

    // 同步等待一个信号
    Signal wait(const std::vector<Signal>& signals) {
        std::promise<Signal> promise;
        auto future = promise.get_future();

        // 注册一个临时 handler
        auto handler = [&promise](Signal sig) {
            promise.set_value(sig);
        };
        HandlerId id = notify(handler, signals);

        // 阻塞等待
        Signal received = future.get();

        // 清理临时 handler
        stop(id);
        return received;
    }

private:
    struct HandlerInfo {
        HandlerId id;
        SignalHandler callback;
        std::vector<Signal> signals;
    };

    void reconfigureLocked() {
        // 计算当前需要监听的信号集合（所有 handler 关心的信号）
        std::unordered_set<int> activeSignals;
        for (const auto& pair : m_sigToHandlers) {
            activeSignals.insert(pair.first);
        }
        // 加入唤醒信号
        activeSignals.insert(m_wakeupSig);

        // 更新全局阻塞掩码：阻塞所有 activeSignals
        sigset_t blockSet;
        sigemptyset(&blockSet);
        for (int sig : activeSignals) {
            sigaddset(&blockSet, sig);
        }
        pthread_sigmask(SIG_BLOCK, &blockSet, nullptr);

        // 重启后台线程
        restartThreadLocked(activeSignals);
    }

    void restartThreadLocked(const std::unordered_set<int>& activeSignals) {
        // 停止旧线程
        if (m_thread.joinable()) {
            m_stopFlag = true;
            // 发送唤醒信号使 sigwait 返回
            pthread_kill(m_thread.native_handle(), m_wakeupSig);
            m_thread.join();
            m_stopFlag = false;
        }
        m_running = false;

        // 如果没有活跃信号，不启动线程
        if (activeSignals.empty()) {
            return;
        }

        // 启动新线程
        m_running = true;
        m_thread = std::thread([this, activeSignals]() {
            // 复制一份活跃信号集供本线程使用
            sigset_t waitSet;
            sigemptyset(&waitSet);
            for (int sig : activeSignals) {
                sigaddset(&waitSet, sig);
            }

            while (!m_stopFlag) {
                int sig;
                int ret = sigwait(&waitSet, &sig);
                if (ret != 0) {
                    // sigwait 失败，继续循环
                    continue;
                }

                // 唤醒信号：用于线程控制，忽略
                if (sig == m_wakeupSig) {
                    continue;
                }

                // 分发信号
                std::vector<SignalHandler> handlersCopy;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    auto it = m_sigToHandlers.find(sig);
                    if (it != m_sigToHandlers.end()) {
                        for (auto hid : it->second) {
                            auto hit = m_handlers.find(hid);
                            if (hit != m_handlers.end()) {
                                handlersCopy.push_back(hit->second.callback);
                            }
                        }
                    }
                }
                // 在锁外调用回调
                Signal sigEnum = static_cast<Signal>(sig);
                for (auto& h : handlersCopy) {
                    h(sigEnum);
                }
            }
        });
    }

    void stopAllInternal() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_handlers.clear();
        m_sigToHandlers.clear();
        if (m_thread.joinable()) {
            m_stopFlag = true;
            pthread_kill(m_thread.native_handle(), m_wakeupSig);
            m_thread.join();
            m_stopFlag = false;
        }
    }

private:
    std::mutex m_mutex;
    std::unordered_map<HandlerId, HandlerInfo> m_handlers;
    std::unordered_map<int, std::unordered_set<HandlerId>> m_sigToHandlers;
    HandlerId m_nextId;
    std::thread m_thread;
    std::atomic<bool> m_stopFlag;
    std::atomic<bool> m_running;
    int m_wakeupSig;
};

inline SignalManager& getManager() {
    static SignalManager manager;
    return manager;
}

// ---------- 公开 API ----------
inline int Notify(SignalHandler handler, const std::vector<Signal>& signals) {
    return getManager().notify(std::move(handler), signals);
}

inline void Stop(int notifyId) { getManager().stop(notifyId); }

inline void StopAll() { getManager().stopAll(); }

inline void Reset(const std::vector<Signal>& signals) { getManager().reset(signals); }

inline void Ignore(const std::vector<Signal>& signals) { getManager().ignore(signals); }

inline Signal Wait(const std::vector<Signal>& signals) { return getManager().wait(signals); }

inline std::string SignalName(Signal sig) {
    switch (sig) {
        case Signal::INT:
            return "INT";
        case Signal::TERM:
            return "TERM";
        case Signal::KILL:
            return "KILL";
        case Signal::HUP:
            return "HUP";
        case Signal::QUIT:
            return "QUIT";
        case Signal::USR1:
            return "USR1";
        case Signal::USR2:
            return "USR2";
        case Signal::PIPE:
            return "PIPE";
        case Signal::ALRM:
            return "ALRM";
        case Signal::CHLD:
            return "CHLD";
        default:
            return "UNKNOWN";
    }
}


}  // namespace signal

inline std::ostream& operator<<(std::ostream& os, const Signal& signal) {
    os << signal::SignalName(signal);
    return os;
}

}  // namespace bre::os
