#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <thread>

#include "breutil/os.hpp"

using namespace bre::os;

struct ArgsInitializer {
    ArgsInitializer() {
        int argc = boost::unit_test::framework::master_test_suite().argc;
        char** argv = boost::unit_test::framework::master_test_suite().argv;
        InitArgs(argc, argv);
    }
};
BOOST_TEST_GLOBAL_FIXTURE(ArgsInitializer);

BOOST_AUTO_TEST_SUITE(OsTestSuite)

// 测试进程 ID 相关函数
BOOST_AUTO_TEST_CASE(test_pid) {
    BOOST_TEST_MESSAGE("Testing PID/PPID/UID/GID");
    int pid = GetPID();
    BOOST_CHECK_GT(pid, 0);
    BOOST_TEST_MESSAGE("PID = " << pid);

    int ppid = GetPPID();
    BOOST_CHECK_GE(ppid, 0);
    BOOST_TEST_MESSAGE("PPID = " << ppid);

    int uid = GetUID();
    int gid = GetGID();
#ifdef _WIN32
    BOOST_CHECK_EQUAL(uid, -1);
    BOOST_CHECK_EQUAL(gid, -1);
    BOOST_TEST_MESSAGE("UID/GID unsupported on Windows");
#else
    BOOST_CHECK_GE(uid, 0);
    BOOST_CHECK_GE(gid, 0);
    BOOST_TEST_MESSAGE("UID = " << uid << ", GID = " << gid);
#endif
}

// 测试文件与路径
BOOST_AUTO_TEST_CASE(test_paths) {
    BOOST_TEST_MESSAGE("Testing Getwd, TempDir, Executable");
    std::string wd;
    BOOST_REQUIRE_NO_THROW(wd = Getwd());
    BOOST_CHECK(!wd.empty());
    BOOST_TEST_MESSAGE("Working directory: " << wd);

    std::string temp;
    BOOST_REQUIRE_NO_THROW(temp = TempDir());
    BOOST_CHECK(!temp.empty());
    BOOST_TEST_MESSAGE("Temp directory: " << temp);

    std::string exe;
    BOOST_REQUIRE_NO_THROW(exe = Executable());
    BOOST_CHECK(!exe.empty());
    BOOST_TEST_MESSAGE("Executable path: " << exe);
}

// 测试命令行参数
BOOST_AUTO_TEST_CASE(test_args) {
    BOOST_TEST_MESSAGE("Testing Args");
    const auto& args = Args();
    BOOST_CHECK_GE(args.size(), 1);
    BOOST_TEST_MESSAGE("argc = " << args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        BOOST_TEST_MESSAGE("argv[" << i << "] = " << args[i]);
    }
}

// 测试环境变量
BOOST_AUTO_TEST_CASE(test_env) {
    BOOST_TEST_MESSAGE("Testing environment variables");
    // 设置一个测试变量
    const std::string testKey = "BREUTIL_TEST_VAR";
    const std::string testValue = "hello_test";

    BOOST_REQUIRE_NO_THROW(Setenv(testKey, testValue));
    BOOST_TEST_MESSAGE("Setenv " << testKey << "=" << testValue);

    std::string value;
    bool found = LookupEnv(testKey, &value);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(value, testValue);
    BOOST_TEST_MESSAGE("LookupEnv found: " << value);

    // 测试 Getenv
    std::string val = Getenv(testKey);
    BOOST_CHECK_EQUAL(val, testValue);

    // 测试 Environ
    auto envMap = Environ();
    BOOST_CHECK(envMap.find(testKey) != envMap.end());
    BOOST_CHECK_EQUAL(envMap[testKey], testValue);
    BOOST_TEST_MESSAGE("Environ size = " << envMap.size());

    // 删除变量
    BOOST_REQUIRE_NO_THROW(Unsetenv(testKey));
    found = LookupEnv(testKey, &value);
    BOOST_CHECK(!found);
    BOOST_TEST_MESSAGE("Unsetenv succeeded");

    // 测试不存在的变量
    value.clear();
    found = LookupEnv("THIS_VAR_SHOULD_NOT_EXIST_XYZ123", &value);
    BOOST_CHECK(!found);
    BOOST_CHECK(value.empty());
}

// 测试系统信息
BOOST_AUTO_TEST_CASE(test_sysinfo) {
    BOOST_TEST_MESSAGE("Testing Hostname and Getpagesize");
    std::string hostname;
    BOOST_REQUIRE_NO_THROW(hostname = Hostname());
    BOOST_CHECK(!hostname.empty());
    BOOST_TEST_MESSAGE("Hostname: " << hostname);

    int pageSize = Getpagesize();
    BOOST_CHECK_GT(pageSize, 0);
    BOOST_TEST_MESSAGE("Page size: " << pageSize << " bytes");
}

// 测试临时目录的可写性（可选）
BOOST_AUTO_TEST_CASE(test_tempdir_writable) {
    std::string tempDir = TempDir();
    std::string testFile = tempDir + "breutil_test.tmp";
    std::ofstream ofs(testFile);
    if (ofs.is_open()) {
        ofs << "test";
        ofs.close();
        std::remove(testFile.c_str());
        BOOST_TEST_MESSAGE("Temp directory is writable");
    } else {
        BOOST_TEST_MESSAGE("Temp directory not writable (test skipped)");
        BOOST_WARN_MESSAGE(false, "Cannot write to temp directory");
    }
}

BOOST_AUTO_TEST_SUITE_END()