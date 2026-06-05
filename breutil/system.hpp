#pragma once


#if defined(__APPLE__)
#include <sched.h>
#include <unistd.h>
#include <pthread.h>
#include <mach/mach.h>
#elif defined(__linux__)
#include <pthread.h>
#include <mach/mach.h>
#elif defined(_WIN32)
#include <windows.h>
#include <process.h>
#endif

namespace bre
{
// 针对POSIX线程的实时优先级设置
#if defined(__APPLE__)
inline int set_realtime_priority() {
    mach_port_t thread = pthread_mach_thread_np(pthread_self());
    thread_time_constraint_policy_data_t policy;
    policy.period = 0;
    policy.computation = 5'000; // 5 ms
    policy.constraint = 10'000; // 10 ms
    policy.preemptible = 0; // 不可抢占

    kern_return_t result = thread_policy_set(thread, THREAD_TIME_CONSTRAINT_POLICY, (thread_policy_t)&policy, THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    return result;
    // mach_port_deallocate(mach_task_self(), thread);
}
#elif defined(__linux__)

inline int set_realtime_priority() {
    struct sched_param param;
    param.sched_priority = 99; // 1~99, 99为最高优先级
    return pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
}
#elif defined(_WIN32)
inline int set_realtime_priority() {
    return 0;
}
#endif


} // namespace bre

