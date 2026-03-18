#define BOOST_TEST_MODULE ThreadPoolTest
#include <atomic>
#include <boost/test/included/unit_test.hpp>
#include <chrono>
#include <thread>

#include "breutil/thread_pool.hpp"


BOOST_AUTO_TEST_SUITE(ThreadPoolTestSuite)


BOOST_AUTO_TEST_CASE(test_basic_submit) {
    bre::ThreadPool pool(4);

    std::atomic<int> counter{0};

    for (int i = 0; i < 100; ++i) {
        pool.Enqueue([&counter]() {
            ++counter;
        });
    }

    pool.WaitAll();
    BOOST_CHECK_EQUAL(counter.load(), 100);
}

BOOST_AUTO_TEST_CASE(test_future_result) {
    bre::ThreadPool pool(2);

    auto future = pool.Enqueue([]() {
        return 42;
    });
    BOOST_CHECK_EQUAL(future.get(), 42);
}

BOOST_AUTO_TEST_CASE(test_multiple_futures) {
    bre::ThreadPool pool(4);

    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.Enqueue([i]() {
            return i * i;
        }));
    }

    for (int i = 0; i < 10; ++i) {
        BOOST_CHECK_EQUAL(futures[i].get(), i * i);
    }
}

BOOST_AUTO_TEST_CASE(test_exception_handling) {
    bre::ThreadPool pool(2);

    auto future = pool.Enqueue([]() -> int {
        throw std::runtime_error("test exception");
    });

    BOOST_CHECK_THROW(future.get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_thread_count) {
    bre::ThreadPool pool(8);
    BOOST_CHECK_EQUAL(pool.GetThreadCount(), 8);
}

BOOST_AUTO_TEST_CASE(test_wait_all) {
    bre::ThreadPool pool(4);

    std::atomic<int> counter{0};

    for (int i = 0; i < 100; ++i) {
        pool.Enqueue([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ++counter;
        });
    }

    pool.WaitAll();
    BOOST_CHECK_EQUAL(counter.load(), 100);
    BOOST_CHECK_EQUAL(pool.GetQueueSize(), 0);
}

BOOST_AUTO_TEST_CASE(test_concurrent_submit) {
    bre::ThreadPool pool(4);

    std::atomic<int> counter{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < 10; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < 100; ++i) {
                pool.Enqueue([&counter]() {
                    ++counter;
                });
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    pool.WaitAll();
    BOOST_CHECK_EQUAL(counter.load(), 1000);
}

BOOST_AUTO_TEST_CASE(test_queue_size) {
    bre::ThreadPool pool(1);  // 只用1个线程

    std::promise<void> task_started;
    std::promise<void> continue_task;
    auto task_started_future = task_started.get_future();

    // 提交一个受控的任务
    auto future = pool.Enqueue([&]() {
        task_started.set_value();           // 通知主线程任务已开始
        continue_task.get_future().wait();  // 等待主线程允许继续
    });

    // 等待任务开始执行
    task_started_future.wait();

    // 此时队列应该是空的，因为任务已被取出执行
    BOOST_CHECK_EQUAL(pool.GetQueueSize(), 0);

    // 提交更多任务到队列
    for (int i = 0; i < 10; ++i) {
        pool.Enqueue([]() {
        });
    }

    // 现在队列中应该有 10 个等待的任务
    BOOST_CHECK_EQUAL(pool.GetQueueSize(), 10);

    // 让第一个任务完成
    continue_task.set_value();

    // 等待所有任务完成
    pool.WaitAll();
    BOOST_CHECK_EQUAL(pool.GetQueueSize(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
