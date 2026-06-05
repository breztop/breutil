#include <boost/test/unit_test.hpp>

#include "breutil/os/user.hpp"

using namespace bre::os::user;

BOOST_AUTO_TEST_SUITE(OSUserInfoTestSuite)

static inline bool is_windows() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

// 测试当前用户信息（始终存在）
BOOST_AUTO_TEST_CASE(test_current_user) {
    BOOST_REQUIRE_NO_THROW({
        Info info = Current();
        BOOST_CHECK(!info.Username.empty());
        BOOST_CHECK(!info.Uid.empty());
        BOOST_CHECK(!info.Gid.empty());
        // Windows 上主目录也可能存在，但某些服务账户可能为空
        BOOST_CHECK(!info.HomeDir.empty());
    });
}

// 测试通过用户名查找当前用户，结果应与 Current() 一致
BOOST_AUTO_TEST_CASE(test_lookup_current_user_by_name) {
    Info cur = Current();
    BOOST_REQUIRE_NO_THROW({
        Info byName = Lookup(cur.Username);
        BOOST_CHECK_EQUAL(byName.Username, cur.Username);
        BOOST_CHECK_EQUAL(byName.Uid, cur.Uid);
        BOOST_CHECK_EQUAL(byName.Gid, cur.Gid);
        BOOST_CHECK_EQUAL(byName.HomeDir, cur.HomeDir);
    });
}

// 测试通过 UID 查找当前用户
BOOST_AUTO_TEST_CASE(test_lookup_current_user_by_uid) {
    Info cur = Current();
    BOOST_REQUIRE_NO_THROW({
        Info byUid = LookupId(cur.Uid);
        BOOST_CHECK_EQUAL(byUid.Username, cur.Username);
        BOOST_CHECK_EQUAL(byUid.Uid, cur.Uid);
        BOOST_CHECK_EQUAL(byUid.Gid, cur.Gid);
        BOOST_CHECK_EQUAL(byUid.HomeDir, cur.HomeDir);
    });
}

// 测试查找不存在的用户名 -> 必须抛出异常
BOOST_AUTO_TEST_CASE(test_lookup_nonexistent_user) {
    const std::string badName = "this_user_should_never_exist_bilibili_微风中的快乐";
    BOOST_CHECK_THROW(Lookup(badName), std::runtime_error);
}

// 测试查找不存在的 UID -> 必须抛出异常
BOOST_AUTO_TEST_CASE(test_lookup_nonexistent_uid) {
    if (is_windows()) {
        const std::string badSid = "S-1-2-3-4-5-6-7-8-9";
        BOOST_CHECK_THROW(LookupId(badSid), std::runtime_error);
    } else {
        const std::string badUid = "999999";
        BOOST_CHECK_THROW(LookupId(badUid), std::runtime_error);
    }
}

// 测试组查找：通过当前用户的主组 ID 查找组
BOOST_AUTO_TEST_CASE(test_group_lookup_by_gid) {
    Info cur = Current();
    BOOST_REQUIRE_NO_THROW({
        Group grp = LookupGroup(cur.Gid);
        BOOST_CHECK(!grp.Name.empty());
        BOOST_CHECK_EQUAL(grp.Gid, cur.Gid);
    });
}

// 测试组查找：通过组名查找，然后反查 GID 一致性（使用已知存在的组）
BOOST_AUTO_TEST_CASE(test_group_lookup_by_name) {
    // 选择一个已知存在的组名
    std::string knownGroup;
    if (is_windows()) {
        knownGroup = "Administrators";
    } else {
        knownGroup = "root";
    }

    BOOST_REQUIRE_NO_THROW({
        Group grpByName = LookupGroupName(knownGroup);
        BOOST_CHECK(!grpByName.Gid.empty());
        BOOST_CHECK_EQUAL(grpByName.Name, knownGroup);

        // 通过 GID 反查，应得到相同的组名
        Group grpByGid = LookupGroup(grpByName.Gid);
        BOOST_CHECK_EQUAL(grpByGid.Name, grpByName.Name);
    });
}

// 测试查找不存在的组名 -> 抛出异常
BOOST_AUTO_TEST_CASE(test_lookup_nonexistent_group_name) {
    const std::string badGroup = "this_group_should_never_exist_bilibili_微风中的快乐";
    BOOST_CHECK_THROW(LookupGroupName(badGroup), std::runtime_error);
}

// 测试查找不存在的 GID -> 抛出异常
BOOST_AUTO_TEST_CASE(test_lookup_nonexistent_gid) {
    if (is_windows()) {
        const std::string badSid = "S-1-2-3-4-5-6-7-8-9";
        BOOST_CHECK_THROW(LookupGroup(badSid), std::runtime_error);
    } else {
        const std::string badGid = "999999";
        BOOST_CHECK_THROW(LookupGroup(badGid), std::runtime_error);
    }
}

BOOST_AUTO_TEST_SUITE_END()