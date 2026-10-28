#include <array>
#include <boost/test/unit_test.hpp>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <tuple>
#include <vector>

#include "breutil/binary_io.hpp"

BOOST_AUTO_TEST_SUITE(BinaryIoTestSuite)

BOOST_AUTO_TEST_CASE(IntegersUseFixedWidthBigEndianWireBytes) {
    std::vector<std::uint8_t> output{0xAA};
    bre::binary::AppendInteger(output, std::uint8_t{0x12});
    bre::binary::AppendInteger(output, std::uint16_t{0x3456});
    bre::binary::AppendInteger(output, std::uint32_t{0x789ABCDE});
    bre::binary::AppendInteger(output, std::uint64_t{0x0123456789ABCDEF});
    bre::binary::AppendInteger(output, std::int16_t{-2});
    const std::vector<std::uint8_t> expected{
        0xAA, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0x01,
        0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFF, 0xFE,
    };
    BOOST_TEST(output == expected, boost::test_tools::per_element());

    std::size_t offset = 1;
    std::uint8_t byte = 0;
    std::uint16_t word = 0;
    std::uint32_t dword = 0;
    std::uint64_t qword = 0;
    std::int16_t signed_word = 0;
    BOOST_REQUIRE(bre::binary::ReadInteger(expected, offset, byte));
    BOOST_REQUIRE(bre::binary::ReadInteger(expected, offset, word));
    BOOST_REQUIRE(bre::binary::ReadInteger(expected, offset, dword));
    BOOST_REQUIRE(bre::binary::ReadInteger(expected, offset, qword));
    BOOST_REQUIRE(bre::binary::ReadInteger(expected, offset, signed_word));
    BOOST_TEST(byte == 0x12U);
    BOOST_TEST(word == 0x3456U);
    BOOST_TEST(dword == 0x789ABCDEU);
    BOOST_TEST(qword == 0x0123456789ABCDEFULL);
    BOOST_TEST(signed_word == -2);
    BOOST_TEST(offset == expected.size());
}

using IntegerTypes = std::tuple<std::uint8_t, std::int8_t, std::uint16_t, std::int16_t,
                                std::uint32_t, std::int32_t, std::uint64_t, std::int64_t>;

BOOST_AUTO_TEST_CASE_TEMPLATE(IntegerLimitsRoundTrip, Integer, IntegerTypes) {
    const std::array<Integer, 3> values{std::numeric_limits<Integer>::min(), Integer{0},
                                        std::numeric_limits<Integer>::max()};
    std::vector<std::uint8_t> bytes;
    for (const auto value : values) {
        bre::binary::AppendInteger(bytes, value);
    }
    BOOST_TEST(bytes.size() == values.size() * sizeof(Integer));

    std::size_t offset = 0;
    for (const auto expected : values) {
        Integer actual = 0;
        BOOST_REQUIRE(bre::binary::ReadInteger(bytes, offset, actual));
        BOOST_TEST(actual == expected);
    }
    BOOST_TEST(offset == bytes.size());
}

BOOST_AUTO_TEST_CASE(IncompleteAndInvalidReadsPreserveCursorAndDestination) {
    const std::array<std::uint8_t, 4> bytes{0xAA, 0x12, 0x34, 0x56};
    const std::array<std::size_t, 5> offsets{
        1, 3, bytes.size(), bytes.size() + 1, std::numeric_limits<std::size_t>::max(),
    };
    for (const auto original_offset : offsets) {
        auto offset = original_offset;
        std::uint32_t value = 0xDEADBEEF;
        BOOST_TEST(!bre::binary::ReadInteger(bytes, offset, value));
        BOOST_TEST(offset == original_offset);
        BOOST_TEST(value == 0xDEADBEEFU);
    }

    std::size_t offset = 0;
    std::uint8_t value = 0xAB;
    BOOST_TEST(!bre::binary::ReadInteger(std::span<const std::uint8_t>{}, offset, value));
    BOOST_TEST(offset == 0U);
    BOOST_TEST(value == 0xABU);
}

BOOST_AUTO_TEST_SUITE_END()
