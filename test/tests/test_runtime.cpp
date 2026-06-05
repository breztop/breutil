// #include <boost/test/unit_test.hpp>
// #include <chrono>
// #include <fstream>
// #include <thread>

// #include "breutil/runtime.hpp"

// using namespace bre::runtime;

// BOOST_AUTO_TEST_SUITE(RuntimeTestSuite)

// // 测试 OS 和 ARCH 编译时常量
// BOOST_AUTO_TEST_CASE(test_os_arch) {
//     const char* os = OS();
//     BOOST_CHECK(os != nullptr);
//     BOOST_CHECK(strlen(os) > 0);
//     // 已知值可能是 windows/linux/darwin/freebsd/unknown
//     BOOST_CHECK(std::string(os) != "");

//     const char* arch = ARCH();
//     BOOST_CHECK(arch != nullptr);
//     BOOST_CHECK(strlen(arch) > 0);
//     // 已知值可能是 amd64/386/arm64/arm/unknown
//     BOOST_CHECK(std::string(arch) != "");
// }


// // 测试 NumGoroutine 返回至少 1（主线程）
// BOOST_AUTO_TEST_CASE(test_num_goroutine) {
//     int threads = NumGoroutine();
//     BOOST_CHECK_GE(threads, 1);
//     // 创建几个线程，再检查
//     std::vector<std::thread> workers;
//     int before = NumGoroutine();
//     for (int i = 0; i < 3; ++i) {
//         workers.emplace_back([]() {
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         });
//     }
//     int after = NumGoroutine();
//     // 由于线程可能尚未全部启动，after 应该 >= before + 2（至少增加2）
//     BOOST_CHECK_GE(after, before + 2);
//     for (auto& t : workers) t.join();
// }

// // 测试 ReadMemStats 填充非零值（至少 Alloc 和 Sys > 0）
// BOOST_AUTO_TEST_CASE(test_read_mem_stats) {
//     MemStats m;
//     ReadMemStats(&m);

//     // 打印内存统计信息
//     BOOST_TEST_MESSAGE("Memory Statistics:");
//     BOOST_TEST_MESSAGE("Alloc: " << m.Alloc);
//     BOOST_TEST_MESSAGE("Sys: " << m.Sys);
//     BOOST_TEST_MESSAGE("HeapAlloc: " << m.HeapAlloc);
//     BOOST_TEST_MESSAGE("HeapSys: " << m.HeapSys);
//     BOOST_TEST_MESSAGE("HeapInuse: " << m.HeapInuse);
//     BOOST_TEST_MESSAGE("TotalAlloc: " << m.TotalAlloc << "\n");

//     // 原来的断言保持不变
//     BOOST_CHECK_GT(m.Alloc, 0);
//     BOOST_CHECK_GT(m.Sys, 0);
//     BOOST_CHECK_GT(m.HeapAlloc, 0);
//     BOOST_CHECK_GT(m.HeapSys, 0);
//     BOOST_CHECK_GT(m.HeapInuse, 0);
//     BOOST_CHECK_GT(m.TotalAlloc, 0);
// }

// // 测试 Caller 的基本功能
// BOOST_AUTO_TEST_CASE(test_caller) {
//     // 定义一个内层函数，调用 Caller 并检查返回字符串不为空
//     auto func = []() -> std::string {
//         return Caller(0);  // 0 跳过自身，返回 func 的调用者（即 test_caller 中的调用点）
//     };
//     std::string caller = func();
//     BOOST_CHECK(!caller.empty());
//     // 应包含函数名或描述（至少不全是 "?"）
//     BOOST_CHECK(caller != "?");
//     // 检查是否能返回源文件信息（如果编译器支持）
//     // 无法强制，但至少字符串非空
// }

// BOOST_AUTO_TEST_CASE(test_caller_with_functions) {
//     auto get_caller = [](int skip) {
//         return Caller(skip);
//     };
//     std::string skip0 = get_caller(0);
//     std::string skip1 = get_caller(1);
//     std::string skip2 = get_caller(2);
//     BOOST_CHECK_NE(skip0, skip1);
//     BOOST_CHECK_NE(skip1, skip2);
// }

// // 测试 Stack 生成非空字符串
// BOOST_AUTO_TEST_CASE(test_stack) {
//     std::string buf;
//     Stack(buf);
//     BOOST_CHECK(!buf.empty());
//     // 应该包含至少一行（通常 #0 等）
//     BOOST_CHECK(buf.find("#0") != std::string::npos || buf.find("no stack") ==
//     std::string::npos);
//     // 测试 all=true（虽然忽略，但不应崩溃）
//     std::string buf2;
//     Stack(buf2);
//     BOOST_CHECK(!buf2.empty());

//     BOOST_TEST_MESSAGE("Stack trace output:\n" << buf);
// }

// // 测试 Stack 的格式正确性（每行以 # 开头）
// BOOST_AUTO_TEST_CASE(test_stack_format) {
//     std::string buf;
//     Stack(buf);
//     std::istringstream iss(buf);
//     std::string line;
//     int lineCount = 0;
//     while (std::getline(iss, line)) {
//         if (line.empty()) continue;
//         if (lineCount == 0 && line == "no stack") break;  // 允许无栈情况
//         BOOST_CHECK(line[0] == '#');
//         lineCount++;
//     }
//     // 正常情况下至少有一行
//     if (lineCount == 0 && buf != "no stack\n") {
//         BOOST_FAIL("Stack output has no lines");
//     }
// }

// BOOST_AUTO_TEST_SUITE_END()
