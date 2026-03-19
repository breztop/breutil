#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


namespace bre {

/**
 * @class Base64
 * @brief Provides methods for Base64 encoding and decoding.
 *
 * The `Base64` class includes static methods for encoding data to Base64 format
 * and decoding Base64-encoded data. It also provides utility methods to validate
 * Base64 characters and encoded strings.
 *
 * @note This implementation uses a static Base64 table for encoding and a decode
 * table for decoding. It supports padding and ensures proper validation of Base64
 * strings.
 *
 * @details
 *  - `IsBase64Char`: Checks if a character is a valid Base64 character.
 *  - `IsBase64Encoded`: Validates if a string is properly Base64-encoded.
 *  - `Encode`: Encodes a string to Base64 format.
 *  - `Decode`: Decodes a Base64-encoded string.
 */
class Base64 {
public:
    /**
     * @brief Checks if a character is a valid Base64 character.
     *
     * @param ch The character to check.
     * @return true if the character is valid, false otherwise.
     */
    static bool IsBase64Char(char ch) {
        return (('A' <= ch) && (ch <= 'Z')) || (('a' <= ch) && (ch <= 'z')) ||
               (('0' <= ch) && (ch <= '9')) || (ch == '+') || (ch == '/');
    }

    /**
     * @brief Validates if a string is properly Base64-encoded.
     *
     * @param str The string to validate.
     * @return true if the string is valid Base64, false otherwise.
     */
    static bool IsBase64Encoded(std::string_view str) {
        if (str.empty()) return true;

        // 1. 检查长度必须是 4 的倍数
        if (str.size() % 4 != 0) {
            return false;
        }

        size_t padding_count = 0;
        size_t n = str.size();

        for (size_t i = 0; i < n; ++i) {
            char ch = str[i];

            if (ch == '=') {
                // 2. 检查填充字符是否只出现在末尾
                // 填充只能在最后 2 个位置
                if (i < n - 2) {
                    return false;
                }
                // 如果是倒数第二个字符是 '=', 则倒数第一个也必须是 '='
                if (i == n - 2 && str[n - 1] != '=') {
                    return false;
                }
                padding_count++;
            } else if (!IsBase64Char(ch)) {
                // 3. 检查非填充字符的合法性
                return false;
            }
        }

        // 填充字符不能超过 2 个
        return padding_count <= 2;
    }

    /**
     * @brief Encodes a string to Base64 format.
     *
     * @param data The string to encode.
     * @return The Base64-encoded string.
     */
    static inline std::string Encode(std::string_view data) {
        std::string result;
        Encode_from_array(data.data(), data.size(), result);
        return result;
    }

    /**
     * @brief Decodes a Base64-encoded string.
     *
     * @param data The Base64-encoded string to decode.
     * @param result The output string to store the decoded result.
     */
    static inline std::string Decode(std::string_view data) {
        std::string result;
        Decode_from_array(data.data(), data.size(), result);
        return result;
    }

    /**
     * @brief Encodes a string to Base64 format.
     *
     * @param data The string to encode.
     * @param result The output string to store the encoded result.
     */
    static inline bool Decode(std::string_view data, std::string& result) {
        return Decode_from_array(data.data(), data.size(), result);
    }
    /**
     * @brief Encodes a string to Base64 format.
     *
     * @param data The string to encode.
     * @param result The output vector to store the encoded result.
     */
    static inline bool Decode(std::string_view data, std::vector<char>& result) {
        return Decode_from_array(data.data(), data.size(), result);
    }
    /**
     * @brief Encodes a string to Base64 format.
     *
     * @param data The string to encode.
     * @param len The length of the string.
     * @param result The output vector to store the encoded result.
     */
    static void Encode_from_array(const void* data, size_t len, std::string& result) {
        result.clear();
        result.resize(((len + 2) / 3) * 4);
        const unsigned char* byte_data = static_cast<const unsigned char*>(data);

        unsigned char c;
        size_t i = 0;
        size_t dest_ix = 0;
        while (i < len) {
            c = (byte_data[i] >> 2) & 0x3f;
            result[dest_ix++] = base64_table[c];

            c = (byte_data[i] << 4) & 0x3f;
            if (++i < len) {
                c |= (byte_data[i] >> 4) & 0x0f;
            }
            result[dest_ix++] = base64_table[c];

            if (i < len) {
                c = (byte_data[i] << 2) & 0x3f;
                if (++i < len) {
                    c |= (byte_data[i] >> 6) & 0x03;
                }
                result[dest_ix++] = base64_table[c];
            } else {
                result[dest_ix++] = '=';
            }

            if (i < len) {
                c = byte_data[i] & 0x3f;
                result[dest_ix++] = base64_table[c];
                ++i;
            } else {
                result[dest_ix++] = '=';
            }
        }
    }
    /**
     * @brief Decodes a Base64-encoded string.
     *
     * @param data The Base64-encoded string to decode.
     * @param len The length of the string.
     * @param result The output vector to store the decoded result.
     * @return true if decoding was successful, false otherwise.
     */
    static bool Decode_from_array(const char* data, size_t len, std::string& result) {
        return decode_from_array_template<std::string>(data, len, result);
    }

    /**
     * @brief Decodes a Base64-encoded string.
     *
     * @param data The Base64-encoded string to decode.
     * @param len The length of the string.
     * @param result The output vector to store the decoded result.
     * @return true if decoding was successful, false otherwise.
     */
    static bool Decode_from_array(const char* data, size_t len, std::vector<char>& result) {
        return decode_from_array_template<std::vector<char>>(data, len, result);
    }

    /**
     * @brief Decodes a Base64-encoded string.
     *
     * @param data The Base64-encoded string to decode.
     * @param len The length of the string.
     * @param result The output vector to store the decoded result.
     * @return true if decoding was successful, false otherwise.
     */
    static bool Decode_from_array(const char* data, size_t len, std::vector<uint8_t>& result) {
        return decode_from_array_template<std::vector<uint8_t>>(data, len, result);
    }

private:
    inline static const unsigned char pd = 0xFD;  // 填充
    inline static const unsigned char sp = 0xFE;  // 空格
    inline static const unsigned char il = 0xFF;  // 非法字符

    inline static const char base64_table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    inline static const unsigned char decode_table[] = {
        // 0  1  2   3   4   5   6   7   8   9
        il, il, il, il, il, il, il, il, il, sp,  //   0 -   9
        sp, sp, sp, sp, il, il, il, il, il, il,  //  10 -  19
        il, il, il, il, il, il, il, il, il, il,  //  20 -  29
        il, il, sp, il, il, il, il, il, il, il,  //  30 -  39
        il, il, il, 62, il, il, il, 63, 52, 53,  //  40 -  49
        54, 55, 56, 57, 58, 59, 60, 61, il, il,  //  50 -  59
        il, pd, il, il, il, 0,  1,  2,  3,  4,   //  60 -  69
        5,  6,  7,  8,  9,  10, 11, 12, 13, 14,  //  70 -  79
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  //  80 -  89
        25, il, il, il, il, il, il, 26, 27, 28,  //  90 -  99
        29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  // 100 - 109
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  // 110 - 119
        49, 50, 51, il, il, il, il, il, il, il,  // 120 - 129
        il, il, il, il, il, il, il, il, il, il,  // 130 - 139
        il, il, il, il, il, il, il, il, il, il,  // 140 - 149
        il, il, il, il, il, il, il, il, il, il,  // 150 - 159
        il, il, il, il, il, il, il, il, il, il,  // 160 - 169
        il, il, il, il, il, il, il, il, il, il,  // 170 - 179
        il, il, il, il, il, il, il, il, il, il,  // 180 - 189
        il, il, il, il, il, il, il, il, il, il,  // 190 - 199
        il, il, il, il, il, il, il, il, il, il,  // 200 - 209
        il, il, il, il, il, il, il, il, il, il,  // 210 - 219
        il, il, il, il, il, il, il, il, il, il,  // 220 - 229
        il, il, il, il, il, il, il, il, il, il,  // 230 - 239
        il, il, il, il, il, il, il, il, il, il,  // 240 - 249
        il, il, il, il, il, il                   // 250 - 255
    };

    /**
     * @brief Decodes a Base64 character to its corresponding value.
     *
     * @param c The Base64 character to decode.
     * @param qbuf The buffer to store the decoded value.
     * @param qlen The length of the decoded value.
     * @return true if decoding was successful, false otherwise.
     */
    static bool decode_character(unsigned char c, unsigned char* qbuf, size_t& qlen) {
        unsigned char decoded = decode_table[c];
        if (decoded == il) {
            return false;  // Invalid character
        }
        qbuf[qlen++] = decoded;
        return true;
    }


    /**
     * @brief Gets the next quantum of Base64 data.
     *
     * @param data The Base64-encoded string.
     * @param len The length of the string.
     * @param dpos The current position in the string.
     * @param qbuf The buffer to store the decoded value.
     * @param padding_count The count of padding characters encountered.
     * @return The length of the decoded quantum.
     */
    static size_t get_next_quantum(const char* data, size_t len, size_t* dpos, unsigned char* qbuf,
                                   size_t* padding_count) {
        size_t qlen = 0;
        *padding_count = 0;  // 初始化padding_count

        while (*dpos < len && qlen < 4) {
            unsigned char c = static_cast<unsigned char>(data[*dpos]);
            ++(*dpos);

            if (handle_padding(c, padding_count, qbuf, qlen)) {
                continue;
            }

            if (!decode_character(c, qbuf, qlen)) {
                return 0;  // Invalid character
            }
        }

        if (qlen < 4) {
            return 0;  // Insufficient padding
        }

        return qlen;
    }

    static bool handle_padding(unsigned char c, size_t* padding_count, unsigned char* qbuf,
                               size_t& qlen) {
        if (c == '=') {
            qbuf[qlen++] = 0;
            (*padding_count)++;  // 增加padding计数
            return true;
        }
        return false;
    }

    /**
     * @brief Decodes a Base64-encoded string to a specified result type.
     *
     * @tparam T The type of the result (e.g., std::string, std::vector<char>).
     * @param data The Base64-encoded string.
     * @param len The length of the string.
     * @param result The output variable to store the decoded result.
     * @return true if decoding was successful, false otherwise.
     */
    template <typename T> static bool decode_from_array_template(const char* data, size_t len,
                                                                 T& result) {
        result.clear();
        result.reserve(len / 4 * 3);
        if (len == 0) {
            return true;
        }
        if (len % 4 != 0) {
            return false;  // Invalid length
        }

        size_t dpos = 0;
        bool success = true;
        unsigned char qbuf[4];
        size_t padding_count = 0;

        while (dpos < len) {
            size_t qlen = get_next_quantum(data, len, &dpos, qbuf, &padding_count);
            if (qlen != 4) {
                success = false;
                break;
            }

            // 计算三个可能的字节
            unsigned char c1 = (qbuf[0] << 2) | ((qbuf[1] >> 4) & 0x03);
            unsigned char c2 = ((qbuf[1] << 4) & 0xf0) | ((qbuf[2] >> 2) & 0x0f);
            unsigned char c3 = ((qbuf[2] << 6) & 0xc0) | qbuf[3];

            // 根据padding_count决定输出多少字节
            switch (padding_count) {
                case 0:
                    result.push_back(c1);
                    result.push_back(c2);
                    result.push_back(c3);
                    break;
                case 1:
                    result.push_back(c1);
                    result.push_back(c2);
                    break;
                case 2:
                    result.push_back(c1);
                    break;
                default:
                    success = false;  // 不应该发生的情况
                    break;
            }
        }

        return success;
    }
};

}  // namespace bre