#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "breutil/encoding/base64.hpp"

using namespace bre;

BOOST_AUTO_TEST_SUITE(Base64TestSuite)

BOOST_AUTO_TEST_CASE(test_base64_base_functions) {
    // Test IsBase64Char
    BOOST_CHECK(Base64::IsBase64Char('A') == true);
    BOOST_CHECK(Base64::IsBase64Char('Z') == true);
    BOOST_CHECK(Base64::IsBase64Char('a') == true);
    BOOST_CHECK(Base64::IsBase64Char('z') == true);
    BOOST_CHECK(Base64::IsBase64Char('0') == true);
    BOOST_CHECK(Base64::IsBase64Char('9') == true);
    BOOST_CHECK(Base64::IsBase64Char('+') == true);
    BOOST_CHECK(Base64::IsBase64Char('/') == true);
    BOOST_CHECK(Base64::IsBase64Char('=') == false);
    BOOST_CHECK(Base64::IsBase64Char('!') == false);
    BOOST_CHECK(Base64::IsBase64Char('@') == false);
    BOOST_CHECK(Base64::IsBase64Char(' ') == false);

    // Test IsBase64Encoded
    BOOST_CHECK(Base64::IsBase64Encoded("TWFu") == true);
    BOOST_CHECK(Base64::IsBase64Encoded("TWE=") == true);
    BOOST_CHECK(Base64::IsBase64Encoded("TQ==") == true);
    BOOST_CHECK(Base64::IsBase64Encoded("TWF==") == false);
    BOOST_CHECK(Base64::IsBase64Encoded("TWFu===") == false);
    BOOST_CHECK(Base64::IsBase64Encoded("TWFu!") == false);
    BOOST_CHECK(Base64::IsBase64Encoded("") == true);
    BOOST_CHECK(Base64::IsBase64Encoded("A") == false);
    BOOST_CHECK(Base64::IsBase64Encoded("AB") == false);
    BOOST_CHECK(Base64::IsBase64Encoded("ABC") == false);
}

BOOST_AUTO_TEST_CASE(test_encode) {
    BOOST_CHECK_EQUAL(Base64::Encode("Man"), "TWFu");
    BOOST_CHECK_EQUAL(Base64::Encode("Ma"), "TWE=");
    BOOST_CHECK_EQUAL(Base64::Encode("M"), "TQ==");
    BOOST_CHECK_EQUAL(Base64::Encode(""), "");
    BOOST_CHECK_EQUAL(Base64::Encode("A"), "QQ==");
    BOOST_CHECK_EQUAL(Base64::Encode("AB"), "QUI=");
    BOOST_CHECK_EQUAL(Base64::Encode("ABC"), "QUJD");
    BOOST_CHECK_EQUAL(Base64::Encode("Hello, World!"), "SGVsbG8sIFdvcmxkIQ==");
}

BOOST_AUTO_TEST_CASE(test_decode) {
    BOOST_CHECK_EQUAL(Base64::Decode("TWFu"), "Man");
    BOOST_CHECK_EQUAL(Base64::Decode("TWE="), "Ma");
    BOOST_CHECK_EQUAL(Base64::Decode("TQ=="), "M");
    BOOST_CHECK_EQUAL(Base64::Decode(""), "");
    BOOST_CHECK_EQUAL(Base64::Decode("QQ=="), "A");
    BOOST_CHECK_EQUAL(Base64::Decode("QUI="), "AB");
    BOOST_CHECK_EQUAL(Base64::Decode("QUJD"), "ABC");
    BOOST_CHECK_EQUAL(Base64::Decode("SGVsbG8sIFdvcmxkIQ=="), "Hello, World!");
}

BOOST_AUTO_TEST_CASE(test_decode_with_output_param) {
    std::string decoded_result;
    BOOST_CHECK(Base64::Decode("TWFu", decoded_result) == true);
    BOOST_CHECK_EQUAL(decoded_result, "Man");

    decoded_result.clear();
    BOOST_CHECK(Base64::Decode("SGVsbG8sIFdvcmxkIQ==", decoded_result) == true);
    BOOST_CHECK_EQUAL(decoded_result, "Hello, World!");

    // Test invalid Base64
    std::string invalid_result;
    BOOST_CHECK(Base64::Decode("TWFu!", invalid_result) == false);
    BOOST_CHECK(Base64::Decode("TWFu===", invalid_result) == false);
    BOOST_CHECK(Base64::Decode("TWF==", invalid_result) == false);
}

BOOST_AUTO_TEST_CASE(test_decode_with_vector) {
    std::vector<char> decoded_vector_result;
    BOOST_CHECK(Base64::Decode("TWFu", decoded_vector_result) == true);
    BOOST_CHECK(std::string(decoded_vector_result.begin(), decoded_vector_result.end()) == "Man");

    decoded_vector_result.clear();
    BOOST_CHECK(Base64::Decode("SGVsbG8sIFdvcmxkIQ==", decoded_vector_result) == true);
    BOOST_CHECK(std::string(decoded_vector_result.begin(), decoded_vector_result.end()) ==
                "Hello, World!");
}

BOOST_AUTO_TEST_CASE(test_roundtrip) {
    std::vector<std::string> test_strings = {
        "",
        "A",
        "AB",
        "ABC",
        "ABCD",
        "Hello, World!",
        "1234567890",
        "!@#$%^&*()",
        "The quick brown fox jumps over the lazy dog",
        "微风中的快乐",
        "\x00\x01\x02\x03",
        "\xFF\xFE\xFD\xFC",
    };

    for (const auto& original : test_strings) {
        std::string encoded = Base64::Encode(original);
        std::string decoded = Base64::Decode(encoded);
        BOOST_CHECK_EQUAL(original, decoded);

        // Also test with output parameter
        std::string decoded2;
        BOOST_CHECK(Base64::Decode(encoded, decoded2) == true);
        BOOST_CHECK_EQUAL(original, decoded2);

        // Test with vector
        std::vector<char> decoded_vec;
        BOOST_CHECK(Base64::Decode(encoded, decoded_vec) == true);
        std::string decoded_vec_str(decoded_vec.begin(), decoded_vec.end());
        BOOST_CHECK_EQUAL(original, decoded_vec_str);
    }
}

BOOST_AUTO_TEST_CASE(test_random_data) {
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    std::uniform_int_distribution<int> length_dist(0, 1000);
    std::uniform_int_distribution<unsigned char> byte_dist(0, 255);

    for (int i = 0; i < 100; ++i) {  // Reduced for speed
        int length = length_dist(rng);
        std::string original;
        original.reserve(length);

        for (int j = 0; j < length; ++j) {
            original.push_back(static_cast<char>(byte_dist(rng)));
        }

        std::string encoded = Base64::Encode(original);
        BOOST_CHECK(Base64::IsBase64Encoded(encoded) == true);

        std::string decoded = Base64::Decode(encoded);
        BOOST_CHECK_EQUAL(original, decoded);
    }
}

BOOST_AUTO_TEST_CASE(test_edge_cases) {
    // Test very long string
    std::string long_str(100000, 'A');
    std::string encoded = Base64::Encode(long_str);
    std::string decoded = Base64::Decode(encoded);
    BOOST_CHECK_EQUAL(long_str, decoded);

    // Test all possible byte values
    std::string all_bytes;
    for (int i = 0; i < 256; ++i) {
        all_bytes.push_back(static_cast<char>(i));
    }
    std::string encoded_bytes = Base64::Encode(all_bytes);
    std::string decoded_bytes = Base64::Decode(encoded_bytes);
    BOOST_CHECK_EQUAL(all_bytes, decoded_bytes);

    // Test strings that are not multiples of 3
    for (int len = 1; len <= 10; ++len) {
        std::string str(len, 'X');
        std::string encoded = Base64::Encode(str);
        std::string decoded = Base64::Decode(encoded);
        BOOST_CHECK_EQUAL(str, decoded);
    }
}

BOOST_AUTO_TEST_CASE(test_unicode_and_special_chars) {
    // Test with Unicode strings
    std::string unicode_str = "🌍😀🎉";
    std::string encoded = Base64::Encode(unicode_str);
    std::string decoded = Base64::Decode(encoded);
    BOOST_CHECK_EQUAL(unicode_str, decoded);

    // Test with mixed content
    std::string mixed = "Hello 世界! 123 €";
    encoded = Base64::Encode(mixed);
    decoded = Base64::Decode(encoded);
    BOOST_CHECK_EQUAL(mixed, decoded);
}

BOOST_AUTO_TEST_CASE(test_complex) {
    std::string str = "微风中的快乐";
    std::string encoded = Base64::Encode(str);
    BOOST_CHECK(Base64::IsBase64Encoded(encoded) == true);

    std::string decoded;
    BOOST_CHECK(Base64::Decode(encoded, decoded) == true);
    BOOST_CHECK_EQUAL(str, decoded);

    std::vector<char> decoded_vector;
    BOOST_CHECK(Base64::Decode(encoded, decoded_vector) == true);
    BOOST_CHECK(std::string(decoded_vector.begin(), decoded_vector.end()) == str);

    // Test with large random data
    std::string long_str;
    for (int i = 0; i < 100000; ++i) {
        long_str += static_cast<char>(rand() % 256);
    }

    std::string long_encoded = Base64::Encode(long_str);
    std::string long_decoded;
    BOOST_CHECK(Base64::Decode(long_encoded, long_decoded) == true);
    BOOST_CHECK_EQUAL(long_str, long_decoded);
}

BOOST_AUTO_TEST_SUITE_END()