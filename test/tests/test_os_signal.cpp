// #include <boost/test/unit_test.hpp>
// #include <boost/thread.hpp>
// #include <chrono>
// #include <future>
// #include <iostream>

// #include "breutil/os/signal.hpp"

// using namespace bre::os;
// using namespace bre::os::signal;
// using namespace std::chrono_literals;

// BOOST_AUTO_TEST_SUITE(OSSignalTestSuite)


// static inline void sendSignal(Signal sig) { ::raise(static_cast<int>(sig)); }
// static inline void tinyWait() { std::this_thread::sleep_for(10ms); }

// // 测试信号名称转换
// BOOST_AUTO_TEST_CASE(test_signal_name) {
//     BOOST_CHECK_EQUAL(SignalName(Signal::INT), "INT");
//     BOOST_CHECK_EQUAL(SignalName(Signal::TERM), "TERM");
//     BOOST_CHECK_EQUAL(SignalName(Signal::USR1), "USR1");
//     BOOST_CHECK_EQUAL(SignalName(Signal::USR2), "USR2");
// }

// BOOST_AUTO_TEST_CASE(test_reset) {
//     StopAll();
//     tinyWait();

//     std::promise<void> promise;
//     auto future = promise.get_future();
//     int id = Notify(
//         [&promise](Signal) {
//             promise.set_value();
//         },
//         {Signal::USR1});

//     Reset({Signal::USR1});
//     tinyWait();

//     sendSignal(Signal::USR1);
//     auto status = future.wait_for(200ms);
//     BOOST_CHECK(status != std::future_status::ready);
// }

// BOOST_AUTO_TEST_CASE(test_wait_sync) {
//     StopAll();
//     tinyWait();

//     std::thread sender([&]() {
//         std::this_thread::sleep_for(100ms);
//         sendSignal(Signal::USR2);
//     });

//     Signal received = Wait({Signal::USR2});
//     BOOST_CHECK_EQUAL(received, Signal::USR2);
//     sender.join();
// }

// BOOST_AUTO_TEST_CASE(test_notify_and_stop) {
//     StopAll();
//     Reset({Signal::USR1});
//     tinyWait();

//     std::promise<void> promise;
//     auto future = promise.get_future();
//     std::atomic<bool> handlerCalled{false};

//     int id = Notify(
//         [&](Signal sig) {
//             BOOST_CHECK_EQUAL(sig, Signal::USR1);
//             handlerCalled = true;
//             promise.set_value();
//         },
//         {Signal::USR1});

//     std::this_thread::sleep_for(200ms);

//     sendSignal(Signal::USR1);

//     auto status = future.wait_for(1s);
//     if (status != std::future_status::ready) {
//         std::thread waiter([&]() {
//             Signal s = Wait({Signal::USR1});
//             BOOST_CHECK_EQUAL(s, Signal::USR1);
//         });
//         waiter.detach();
//         std::this_thread::sleep_for(500ms);
//     }
//     BOOST_CHECK(status == std::future_status::ready);
//     BOOST_CHECK(handlerCalled);

//     Stop(id);
//     std::promise<void> promise2;
//     auto future2 = promise2.get_future();
//     int id2 = Notify(
//         [&](Signal) {
//             promise2.set_value();
//         },
//         {Signal::USR1});
//     Stop(id2);
//     sendSignal(Signal::USR1);
//     status = future2.wait_for(200ms);
//     BOOST_CHECK(status != std::future_status::ready);
// }

// BOOST_AUTO_TEST_CASE(test_reset_ignore) {
//     StopAll();
//     BOOST_CHECK_NO_THROW(Reset({Signal::USR1}));
//     BOOST_CHECK_NO_THROW(Ignore({Signal::USR2}));
// }


// BOOST_AUTO_TEST_CASE(test_ignore) {
//     StopAll();
//     tinyWait();

//     std::promise<void> promise;
//     auto future = promise.get_future();
//     int id = Notify(
//         [&promise](Signal) {
//             promise.set_value();
//         },
//         {Signal::USR1});

//     Ignore({Signal::USR1});
//     tinyWait();

//     sendSignal(Signal::USR1);
//     auto status = future.wait_for(200ms);
//     BOOST_CHECK(status != std::future_status::ready);

//     Stop(id);
//     Reset({Signal::USR1});
// }

// BOOST_AUTO_TEST_CASE(test_wait) {
//     StopAll();
//     tinyWait();

//     std::thread sender([&]() {
//         std::this_thread::sleep_for(100ms);
//         sendSignal(Signal::USR2);
//     });

//     Signal received = Wait({Signal::USR2});
//     BOOST_CHECK_EQUAL(received, Signal::USR2);
//     sender.join();
// }

// BOOST_AUTO_TEST_CASE(test_wait_cleanup) {
//     StopAll();
//     tinyWait();

//     std::promise<void> permPromise;
//     auto permFuture = permPromise.get_future();
//     int permId = Notify(
//         [&permPromise](Signal) {
//             permPromise.set_value();
//         },
//         {Signal::USR1});

//     std::thread waiter([&]() {
//         Signal s = Wait({Signal::USR2});
//         BOOST_CHECK_EQUAL(s, Signal::USR2);
//     });

//     std::this_thread::sleep_for(50ms);
//     sendSignal(Signal::USR2);
//     waiter.join();

//     sendSignal(Signal::USR1);
//     auto status = permFuture.wait_for(200ms);
//     BOOST_CHECK(status == std::future_status::ready);

//     Stop(permId);
// }

// BOOST_AUTO_TEST_CASE(test_multiple_handlers_same_signal) {
//     StopAll();
//     tinyWait();

//     std::promise<void> promise1, promise2;
//     auto future1 = promise1.get_future();
//     auto future2 = promise2.get_future();

//     int id1 = Notify(
//         [&promise1](Signal) {
//             promise1.set_value();
//         },
//         {Signal::USR1});
//     int id2 = Notify(
//         [&promise2](Signal) {
//             promise2.set_value();
//         },
//         {Signal::USR1});

//     sendSignal(Signal::USR1);
//     auto status1 = future1.wait_for(200ms);
//     auto status2 = future2.wait_for(200ms);
//     BOOST_CHECK(status1 == std::future_status::ready);
//     BOOST_CHECK(status2 == std::future_status::ready);

//     Stop(id1);
//     Stop(id2);
// }

// BOOST_AUTO_TEST_CASE(test_stop_all) {
//     StopAll();
//     tinyWait();

//     std::promise<void> promise;
//     auto future = promise.get_future();
//     int id = Notify(
//         [&promise](Signal) {
//             promise.set_value();
//         },
//         {Signal::USR1});

//     StopAll();
//     tinyWait();

//     sendSignal(Signal::USR1);
//     auto status = future.wait_for(200ms);
//     BOOST_CHECK(status != std::future_status::ready);
// }

// BOOST_AUTO_TEST_SUITE_END()
