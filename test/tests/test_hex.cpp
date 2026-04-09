#include <boost/test/unit_test.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "breutil/encoding/hex.hpp"

using namespace bre;

BOOST_AUTO_TEST_SUITE(HexTestSuite)

// ------------------ ToBytes ------------------

BOOST_AUTO_TEST_CASE(test_hex_to_bytes) {
    // 正常情况
    auto bytes = Hex::ToBytes("48656C6C6F");
    BOOST_CHECK_EQUAL(bytes.size(), 5);
    BOOST_CHECK_EQUAL(bytes[0], 0x48);
    BOOST_CHECK_EQUAL(bytes[1], 0x65);
    BOOST_CHECK_EQUAL(bytes[2], 0x6C);
    BOOST_CHECK_EQUAL(bytes[3], 0x6C);
    BOOST_CHECK_EQUAL(bytes[4], 0x6F);

    // 小写十六进制
    bytes = Hex::ToBytes("48656c6c6f");
    BOOST_CHECK_EQUAL(bytes.size(), 5);
    BOOST_CHECK_EQUAL(bytes[0], 0x48);
    BOOST_CHECK_EQUAL(bytes[4], 0x6F);

    // 空字符串
    bytes = Hex::ToBytes("");
    BOOST_CHECK_EQUAL(bytes.size(), 0);
}

// ------------------ FromBytes ------------------

BOOST_AUTO_TEST_CASE(test_bytes_to_hex) {
    // 正常情况
    std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    std::string hex = Hex::FromBytes(data);
    BOOST_CHECK_EQUAL(hex, "48656C6C6F");

    // 空数据
    data = {};
    hex = Hex::FromBytes(data);
    BOOST_CHECK_EQUAL(hex, "");

    // 单个字节
    data = {0x0A};
    hex = Hex::FromBytes(data);
    BOOST_CHECK_EQUAL(hex, "0A");
}

// ------------------ FormatInt ------------------

BOOST_AUTO_TEST_CASE(test_format_int) {
    // 正数
    std::string hex = Hex::FormatInt(255, true);
    BOOST_CHECK_EQUAL(hex, "FF");

    hex = Hex::FormatInt(16);
    BOOST_CHECK_EQUAL(hex, "10");

    hex = Hex::FormatInt(0);
    BOOST_CHECK_EQUAL(hex, "0");

    hex = Hex::FormatInt(-1);
    BOOST_CHECK_EQUAL(hex, "-1");

    hex = Hex::FormatInt(-255);
    BOOST_CHECK_EQUAL(hex, "-FF");
}

// ------------------ ParseInt ------------------

BOOST_AUTO_TEST_CASE(test_parse_int) {
    // 正常情况
    auto result = Hex::ParseInt("FF");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 255);

    result = Hex::ParseInt("10");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 16);

    result = Hex::ParseInt("0");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 0);


    result = Hex::ParseInt("FFFFFFFF");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 4294967295);

    result = Hex::ParseInt("FFFFFF01");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 4294967041);

    // 小写十六进制
    result = Hex::ParseInt("ff");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 255);

    // 无效输入
    result = Hex::ParseInt("GG");
    BOOST_CHECK(!result.has_value());

    result = Hex::ParseInt("");
    BOOST_CHECK(!result.has_value());

    result = Hex::ParseInt("0x123");
    BOOST_CHECK(!result.has_value());
}

BOOST_AUTO_TEST_SUITE_END()