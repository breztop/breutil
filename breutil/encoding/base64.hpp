#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#ifndef BRE_BASE64_USE_SIMD
#ifdef BRE_USE_SIMD
#define BRE_BASE64_USE_SIMD 1
#else
#define BRE_BASE64_USE_SIMD 0
#endif
#endif

#if BRE_BASE64_USE_SIMD
#include <experimental/simd>
#endif

namespace bre {

namespace detail {

inline constexpr std::uint8_t invalid_base64 = 0xff;
inline constexpr std::uint8_t il = invalid_base64;
inline constexpr std::uint8_t sp = invalid_base64;
inline constexpr std::uint8_t pd = invalid_base64;
inline constexpr std::array<std::uint8_t, 256> base64_decode_table = {
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

}  // namespace detail

/** RFC 4648 Base64 encoder and strict decoder. */
class Base64 {
public:
    static bool IsBase64Char(char ch) noexcept;
    static bool IsBase64Encoded(std::string_view data) noexcept;

    static std::string Encode(std::string_view data);
    static std::string Decode(std::string_view data);
    static bool Decode(std::string_view data, std::string& result);
    static bool Decode(std::string_view data, std::vector<char>& result);

    static void EncodeFromArray(const void* data, std::size_t size, std::string& result);
    static bool DecodeFromArray(const char* data, std::size_t size, std::string& result);
    static bool DecodeFromArray(const char* data, std::size_t size, std::vector<char>& result);
    static bool DecodeFromArray(const char* data, std::size_t size,
                                std::vector<std::uint8_t>& result);

    // Compatibility aliases for the original public API.
    static void Encode_from_array(const void* data, std::size_t size, std::string& result) {
        EncodeFromArray(data, size, result);
    }
    template <typename Output>
    static bool Decode_from_array(const char* data, std::size_t size, Output& result) {
        return DecodeFromArray(data, size, result);
    }

private:
    static constexpr char alphabet_[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    static std::uint8_t decode_value(unsigned char value) noexcept;
    static bool validate(std::string_view data) noexcept;
    template <typename Output>
    static bool decode_to(std::string_view data, Output& result);
};

inline bool Base64::IsBase64Char(char ch) noexcept {
    return detail::base64_decode_table[static_cast<unsigned char>(ch)] != detail::invalid_base64;
}

inline std::uint8_t Base64::decode_value(unsigned char c) noexcept {
    return detail::base64_decode_table[c];
}

inline bool Base64::validate(std::string_view data) noexcept {
    if (data.empty()) return true;
    if (data.size() % 4 != 0) return false;

    std::size_t padding = 0;
    if (data.back() == '=') {
        padding = 1;
        if (data[data.size() - 2] == '=') padding = 2;
    }
    const std::size_t alphabet_size = data.size() - padding;
    if (alphabet_size == 0) return false;

    std::size_t i = 0;
#if BRE_BASE64_USE_SIMD
    using ByteVector = std::experimental::fixed_size_simd<unsigned char, 16>;
    const ByteVector upper_a(static_cast<unsigned char>('A'));
    const ByteVector upper_z(static_cast<unsigned char>('Z'));
    const ByteVector lower_a(static_cast<unsigned char>('a'));
    const ByteVector lower_z(static_cast<unsigned char>('z'));
    const ByteVector digit_0(static_cast<unsigned char>('0'));
    const ByteVector digit_9(static_cast<unsigned char>('9'));
    const ByteVector plus(static_cast<unsigned char>('+'));
    const ByteVector slash(static_cast<unsigned char>('/'));
    for (; i + 16 <= alphabet_size; i += 16) {
        const ByteVector chars([&](auto lane) {
            return static_cast<unsigned char>(data[i + lane]);
        });
        const auto valid =
            ((chars >= upper_a) & (chars <= upper_z)) | ((chars >= lower_a) & (chars <= lower_z)) |
            ((chars >= digit_0) & (chars <= digit_9)) | (chars == plus) | (chars == slash);
        if (!std::experimental::all_of(valid)) return false;
    }
#endif
    for (; i < alphabet_size; ++i) {
        if (!IsBase64Char(data[i])) return false;
    }

    // Padding is legal only in the final quantum. Reject non-canonical pad bits.
    if (padding == 1 && (decode_value(static_cast<unsigned char>(data[data.size() - 2])) & 0x03))
        return false;
    if (padding == 2 && (decode_value(static_cast<unsigned char>(data[data.size() - 3])) & 0x0f))
        return false;
    return true;
}

inline bool Base64::IsBase64Encoded(std::string_view data) noexcept { return validate(data); }

inline std::string Base64::Encode(std::string_view data) {
    std::string result;
    EncodeFromArray(data.data(), data.size(), result);
    return result;
}

inline void Base64::EncodeFromArray(const void* data, std::size_t size, std::string& result) {
    result.clear();
    if (size == 0) return;
    if (data == nullptr) return;

    const auto* input = static_cast<const std::uint8_t*>(data);
    result.resize(((size + 2) / 3) * 4);
    std::size_t source = 0;
    std::size_t target = 0;

    while (source + 3 <= size) {
        const std::uint32_t value = (static_cast<std::uint32_t>(input[source]) << 16) |
                                    (static_cast<std::uint32_t>(input[source + 1]) << 8) |
                                    input[source + 2];
        result[target++] = alphabet_[(value >> 18) & 0x3f];
        result[target++] = alphabet_[(value >> 12) & 0x3f];
        result[target++] = alphabet_[(value >> 6) & 0x3f];
        result[target++] = alphabet_[value & 0x3f];
        source += 3;
    }

    if (source < size) {
        const std::uint32_t value =
            static_cast<std::uint32_t>(input[source]) << 16 |
            (source + 1 < size ? static_cast<std::uint32_t>(input[source + 1]) << 8 : 0);
        result[target++] = alphabet_[(value >> 18) & 0x3f];
        result[target++] = alphabet_[(value >> 12) & 0x3f];
        result[target++] = source + 1 < size ? alphabet_[(value >> 6) & 0x3f] : '=';
        result[target] = '=';
    }
}

template <typename Output>
inline bool Base64::decode_to(std::string_view data, Output& result) {
    result.clear();

    if (!validate(data)) {
        return false;
    }
    if (data.empty()) {
        return true;
    }

    const std::size_t padding = (data.back() == '=') + (data[data.size() - 2] == '=');
    const std::size_t output_size = data.size() / 4 * 3 - padding;
    result.resize(output_size);
    const std::size_t full_end = data.size() - 4;
    std::size_t target = 0;

    for (std::size_t source = 0; source < full_end; source += 4) {
        const std::uint32_t value =
            (static_cast<std::uint32_t>(decode_value(data[source])) << 18) |
            (static_cast<std::uint32_t>(decode_value(data[source + 1])) << 12) |
            (static_cast<std::uint32_t>(decode_value(data[source + 2])) << 6) |
            decode_value(data[source + 3]);
        result[target++] = static_cast<typename Output::value_type>(value >> 16);
        result[target++] = static_cast<typename Output::value_type>(value >> 8);
        result[target++] = static_cast<typename Output::value_type>(value);
    }

    const std::size_t i = full_end;
    const std::uint32_t value =
        (static_cast<std::uint32_t>(decode_value(data[i])) << 18) |
        (static_cast<std::uint32_t>(decode_value(data[i + 1])) << 12) |
        (padding < 2 ? static_cast<std::uint32_t>(decode_value(data[i + 2])) << 6 : 0) |
        (padding == 0 ? decode_value(data[i + 3]) : 0);
    result[target++] = static_cast<typename Output::value_type>(value >> 16);
    if (padding < 2) result[target++] = static_cast<typename Output::value_type>(value >> 8);
    if (padding == 0) result[target] = static_cast<typename Output::value_type>(value);
    return true;
}

inline std::string Base64::Decode(std::string_view data) {
    std::string result;
    decode_to(data, result);
    return result;
}

inline bool Base64::Decode(std::string_view data, std::string& result) {
    return decode_to(data, result);
}

inline bool Base64::Decode(std::string_view data, std::vector<char>& result) {
    return decode_to(data, result);
}

inline bool Base64::DecodeFromArray(const char* data, std::size_t size, std::string& result) {
    if (data == nullptr && size != 0) {
        result.clear();
        return false;
    }
    return decode_to(std::string_view(data == nullptr ? "" : data, size), result);
}

inline bool Base64::DecodeFromArray(const char* data, std::size_t size, std::vector<char>& result) {
    if (data == nullptr && size != 0) {
        result.clear();
        return false;
    }
    return decode_to(std::string_view(data == nullptr ? "" : data, size), result);
}

inline bool Base64::DecodeFromArray(const char* data, std::size_t size,
                                    std::vector<std::uint8_t>& result) {
    if (data == nullptr && size != 0) {
        result.clear();
        return false;
    }
    return decode_to(std::string_view(data == nullptr ? "" : data, size), result);
}

#if 0
// 标准库 SIMD 不提供 Base64 所需的跨通道 3<->4 字节
// 混洗操作。临时数组和标量通道重排导致编码/解码速度慢了约 10 倍。保留此代码以供未来的 std::simd 工作使用；
// 在当前实现中，只有 validate() 使用了 SIMD。

inline void encode_simd_experiment(const std::uint8_t* input, std::size_t size,
                                   std::string& result) {
    using ByteVector = std::experimental::fixed_size_simd<unsigned char, 16>;
    alignas(ByteVector) unsigned char indices[16];
    alignas(ByteVector) unsigned char encoded[16];
    std::size_t source = 0;
    std::size_t target = 0;

    while (source + 12 <= size) {
        for (std::size_t group = 0; group < 4; ++group) {
            const std::size_t offset = source + group * 3;
            const std::uint32_t value =
                (static_cast<std::uint32_t>(input[offset]) << 16) |
                (static_cast<std::uint32_t>(input[offset + 1]) << 8) | input[offset + 2];
            indices[group * 4] = static_cast<unsigned char>((value >> 18) & 0x3f);
            indices[group * 4 + 1] = static_cast<unsigned char>((value >> 12) & 0x3f);
            indices[group * 4 + 2] = static_cast<unsigned char>((value >> 6) & 0x3f);
            indices[group * 4 + 3] = static_cast<unsigned char>(value & 0x3f);
        }

        const ByteVector values(indices, std::experimental::element_aligned);
        ByteVector ascii = values + ByteVector(static_cast<unsigned char>('A'));
        const auto lower = values >= ByteVector(26);
        const auto digit = values >= ByteVector(52);
        std::experimental::where(lower, ascii) =
            values + ByteVector(static_cast<unsigned char>('a' - 26));
        std::experimental::where(digit, ascii) =
            values + ByteVector(static_cast<unsigned char>('0' - 52));
        std::experimental::where(values == ByteVector(62), ascii) =
            ByteVector(static_cast<unsigned char>('+'));
        std::experimental::where(values == ByteVector(63), ascii) =
            ByteVector(static_cast<unsigned char>('/'));
        ascii.copy_to(encoded, std::experimental::element_aligned);
        for (std::size_t lane = 0; lane < 16; ++lane) result[target + lane] = encoded[lane];
        source += 12;
        target += 16;
    }
}

template <typename Output>
inline bool decode_simd_experiment(std::string_view data, Output& result) {
    using ByteVector = std::experimental::fixed_size_simd<unsigned char, 16>;
    alignas(ByteVector) unsigned char mapped[16];
    const ByteVector upper_a(static_cast<unsigned char>('A'));
    const ByteVector upper_z(static_cast<unsigned char>('Z'));
    const ByteVector lower_a(static_cast<unsigned char>('a'));
    const ByteVector lower_z(static_cast<unsigned char>('z'));
    const ByteVector digit_0(static_cast<unsigned char>('0'));
    const ByteVector digit_9(static_cast<unsigned char>('9'));
    const ByteVector plus(static_cast<unsigned char>('+'));
    const ByteVector slash(static_cast<unsigned char>('/'));
    const std::size_t full_end = data.size() - 4;
    std::size_t source = 0;
    std::size_t target = 0;

    for (; source + 16 <= full_end; source += 16) {
        const ByteVector chars([&](auto lane) {
            return static_cast<unsigned char>(data[source + lane]);
        });
        const auto upper = (chars >= upper_a) & (chars <= upper_z);
        const auto lower = (chars >= lower_a) & (chars <= lower_z);
        const auto digit = (chars >= digit_0) & (chars <= digit_9);
        const auto is_plus = chars == plus;
        const auto is_slash = chars == slash;
        if (!std::experimental::all_of(upper | lower | digit | is_plus | is_slash)) return false;

        ByteVector values = chars - upper_a;
        std::experimental::where(lower, values) = chars - lower_a + ByteVector(26);
        std::experimental::where(digit, values) = chars - digit_0 + ByteVector(52);
        std::experimental::where(is_plus, values) = ByteVector(62);
        std::experimental::where(is_slash, values) = ByteVector(63);
        values.copy_to(mapped, std::experimental::element_aligned);

        for (std::size_t lane = 0; lane < 16; lane += 4) {
            const std::uint32_t value =
                (static_cast<std::uint32_t>(mapped[lane]) << 18) |
                (static_cast<std::uint32_t>(mapped[lane + 1]) << 12) |
                (static_cast<std::uint32_t>(mapped[lane + 2]) << 6) | mapped[lane + 3];
            result[target++] = static_cast<typename Output::value_type>(value >> 16);
            result[target++] = static_cast<typename Output::value_type>(value >> 8);
            result[target++] = static_cast<typename Output::value_type>(value);
        }
    }
    return true;
}
#endif

}  // namespace bre
