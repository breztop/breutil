#include <boost/test/unit_test.hpp>
#include <cfloat>
#include <climits>
#include <iomanip>
#include <sstream>
#include <string>

#include "breutil/strconv.hpp"

using namespace bre::strconv;

BOOST_AUTO_TEST_SUITE(StrConvTestSuite)

// ------------------ Atoi ------------------

BOOST_AUTO_TEST_CASE(test_atoi) {
    // 正常情况
    auto result = Atoi("123");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 123);

    result = Atoi("-456");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), -456);

    result = Atoi("+789");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 789);

    // 零
    result = Atoi("0");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 0);

    result = Atoi("+0");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 0);

    result = Atoi("-0");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 0);

    // 边界值
    result = Atoi(std::to_string(INT_MAX));
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), INT_MAX);

    result = Atoi(std::to_string(INT_MIN));
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), INT_MIN);

    result = Atoi("2147483648");
    BOOST_CHECK(!result.has_value());

    result = Atoi("-2147483649");  // < INT_MIN
    BOOST_CHECK(!result.has_value());

    // 非法输入
    BOOST_CHECK(!Atoi("abc").has_value());
    BOOST_CHECK(!Atoi("12.34").has_value());
    BOOST_CHECK(!Atoi("1e5").has_value());
    BOOST_CHECK(!Atoi("").has_value());
    BOOST_CHECK(!Atoi("+").has_value());
    BOOST_CHECK(!Atoi("-").has_value());
    BOOST_CHECK(!Atoi("++123").has_value());
    BOOST_CHECK(!Atoi("--456").has_value());
    BOOST_CHECK(!Atoi("+-789").has_value());
    BOOST_CHECK(!Atoi(" 123").has_value());
    BOOST_CHECK(!Atoi("123 ").has_value());
    BOOST_CHECK(!Atoi("\t-456\n").has_value());
    BOOST_CHECK(!Atoi("12 34").has_value());

    // 前导零
    result = Atoi("00123");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 123);

    result = Atoi("-00456");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), -456);
}

// ------------------ ParseBool ------------------

BOOST_AUTO_TEST_CASE(test_parse_bool) {
    // True cases
    BOOST_CHECK(ParseBool("true").value() == true);
    BOOST_CHECK(ParseBool("True").value() == true);
    BOOST_CHECK(ParseBool("TRUE").value() == true);
    BOOST_CHECK(ParseBool("1").value() == true);
    BOOST_CHECK(ParseBool("t").value() == true);
    BOOST_CHECK(ParseBool("T").value() == true);
    BOOST_CHECK(ParseBool("yes").value() == true);
    BOOST_CHECK(ParseBool("Yes").value() == true);
    BOOST_CHECK(ParseBool("YES").value() == true);

    // False cases
    BOOST_CHECK(ParseBool("false").value() == false);
    BOOST_CHECK(ParseBool("False").value() == false);
    BOOST_CHECK(ParseBool("FALSE").value() == false);
    BOOST_CHECK(ParseBool("0").value() == false);
    BOOST_CHECK(ParseBool("f").value() == false);
    BOOST_CHECK(ParseBool("F").value() == false);
    BOOST_CHECK(ParseBool("no").value() == false);
    BOOST_CHECK(ParseBool("No").value() == false);
    BOOST_CHECK(ParseBool("NO").value() == false);

    // Invalid cases
    BOOST_CHECK(!ParseBool("").has_value());
    BOOST_CHECK(!ParseBool("maybe").has_value());
    BOOST_CHECK(!ParseBool("2").has_value());
    BOOST_CHECK(!ParseBool("y").has_value());
    BOOST_CHECK(!ParseBool("n").has_value());
    BOOST_CHECK(!ParseBool("Tru").has_value());
    BOOST_CHECK(!ParseBool("fals").has_value());
}

// ------------------ FormatBool ------------------

BOOST_AUTO_TEST_CASE(test_format_bool) {
    BOOST_CHECK_EQUAL(FormatBool(true), "true");
    BOOST_CHECK_EQUAL(FormatBool(false), "false");
}

// ------------------ ParseDouble ------------------

BOOST_AUTO_TEST_CASE(test_parse_double) {
    // Normal numbers
    auto result = ParseDouble("123.45");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_CLOSE(result.value(), 123.45, 1e-5);

    result = ParseDouble("-67.89");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_CLOSE(result.value(), -67.89, 1e-5);

    result = ParseDouble("+0.123");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_CLOSE(result.value(), 0.123, 1e-5);

    // Integers should also work
    result = ParseDouble("42");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_CLOSE(result.value(), 42.0, 1e-5);

    result = ParseDouble("-0");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_CLOSE(result.value(), -0.0, 1e-5);

    // Scientific notation (note: std::from_chars for double supports it since C++17)
    result = ParseDouble("1e5");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_CLOSE(result.value(), 100000.0, 1e-5);

    result = ParseDouble("1.23e-4");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_CLOSE(result.value(), 0.000123, 1e-5);

    // Edge values
    result = ParseDouble("0");
    BOOST_CHECK(result.has_value());
    BOOST_CHECK_EQUAL(result.value(), 0.0);


    BOOST_CHECK_EQUAL(ParseDouble("inf").value(), std::numeric_limits<double>::infinity());
    BOOST_CHECK(std::isnan(ParseDouble("nan").value()));


    // Invalid inputs
    BOOST_CHECK(!ParseDouble("").has_value());
    BOOST_CHECK(!ParseDouble("abc").has_value());
    BOOST_CHECK(!ParseDouble("12.34.56").has_value());
    BOOST_CHECK(!ParseDouble("12a34").has_value());
    BOOST_CHECK(!ParseDouble(" 123.45").has_value());  // no whitespace skipping
    BOOST_CHECK(!ParseDouble("123.45 ").has_value());
    BOOST_CHECK(!ParseDouble("++123").has_value());
    BOOST_CHECK(!ParseDouble("--123").has_value());
}

// ------------------ FormatDouble ------------------

BOOST_AUTO_TEST_CASE(test_format_double) {
    std::string def = FormatDouble(123.456789, 'f', 6);
    BOOST_CHECK_EQUAL(def, std::to_string(123.456789));

    // Fixed precision
    BOOST_CHECK_EQUAL(FormatDouble(123.456789, 'f', 2), "123.46");
    BOOST_CHECK_EQUAL(FormatDouble(123.0, 'f', 2), "123.00");
    BOOST_CHECK_EQUAL(FormatDouble(0.1, 'f', 3), "0.100");
    BOOST_CHECK_EQUAL(FormatDouble(-45.6789, 'f', 3), "-45.679");

    // Zero precision
    BOOST_CHECK_EQUAL(FormatDouble(123.456, 'f', 0), "123");

    // Negative zero
    BOOST_CHECK_EQUAL(FormatDouble(-0.0, 'f', 2), "-0.00");

    // Large numbers
    BOOST_CHECK_EQUAL(FormatDouble(1234567.89, 'f', 1), "1234567.9");
}

// ------------------ FormatInt ------------------

BOOST_AUTO_TEST_CASE(test_format_int) {
    // Decimal
    BOOST_CHECK_EQUAL(FormatInt(123), "123");
    BOOST_CHECK_EQUAL(FormatInt(-456), "-456");
    BOOST_CHECK_EQUAL(FormatInt(0), "0");

    // Binary
    BOOST_CHECK_EQUAL(FormatInt(10, 2), "1010");
    BOOST_CHECK_EQUAL(FormatInt(-10, 2), "-1010");

    // Octal
    BOOST_CHECK_EQUAL(FormatInt(64, 8), "100");
    BOOST_CHECK_EQUAL(FormatInt(-64, 8), "-100");

    // Hexadecimal (lowercase)
    BOOST_CHECK_EQUAL(FormatInt(255, 16), "ff");
    BOOST_CHECK_EQUAL(FormatInt(-255, 16), "-ff");
    BOOST_CHECK_EQUAL(FormatInt(0, 16), "0");

    // Edge cases
    BOOST_CHECK_EQUAL(FormatInt(LLONG_MAX, 10), std::to_string(LLONG_MAX));
    BOOST_CHECK_EQUAL(FormatInt(LLONG_MIN, 10), std::to_string(LLONG_MIN));

    // Invalid base (should return empty string per your implementation)
    BOOST_CHECK_EQUAL(FormatInt(10, 1), "");
    BOOST_CHECK_EQUAL(FormatInt(10, 37), "");
}

BOOST_AUTO_TEST_CASE(test_parse_int_uint_format) {
    auto i64 = ParseInt("-123", 10, 64);
    BOOST_CHECK(i64.has_value());
    BOOST_CHECK_EQUAL(i64.value(), -123);

    auto u64 = ParseUint("ff", 16, 64);
    BOOST_CHECK(u64.has_value());
    BOOST_CHECK_EQUAL(u64.value(), 255);

    BOOST_CHECK_EQUAL(Itoa(-789), "-789");
    BOOST_CHECK_EQUAL(FormatUint(255, 16), "ff");
    BOOST_CHECK_EQUAL(ParseUint("100", 2, 64).value(), 4);

    BOOST_CHECK(!ParseInt("", 10, 64).has_value());
    BOOST_CHECK(!ParseUint("-1", 10, 64).has_value());

    BOOST_CHECK(!ParseInt("2147483648", 10, 32).has_value());
    BOOST_CHECK(!ParseInt("-2147483649", 10, 32).has_value());
}


BOOST_AUTO_TEST_CASE(test_quote_unquote) {
    BOOST_CHECK_EQUAL(Quote("a\nb"), "\"a\\nb\"");
    BOOST_CHECK_EQUAL(Unquote("\"a\\nb\"").value(), "a\nb");

    auto qp = QuotedPrefix("\"abc\"rest");
    BOOST_CHECK_EQUAL(qp.first, "\"abc\"");
    BOOST_CHECK(!qp.second.has_value());

    auto uc = UnquoteChar("\\nabc", '\'');
    BOOST_CHECK(std::get<0>(uc) == '\n');
    BOOST_CHECK(std::get<2>(uc) == "abc");
}

BOOST_AUTO_TEST_CASE(test_isgraphic_isprint) {
    BOOST_CHECK(IsPrint(U'a'));
    BOOST_CHECK(!IsPrint(0x1F));
    BOOST_CHECK(IsGraphic(U'a'));
    BOOST_CHECK(!IsGraphic(U'\n'));
}

BOOST_AUTO_TEST_SUITE_END()