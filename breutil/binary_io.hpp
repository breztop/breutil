#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <vector>

namespace bre::binary {

// Protocol integers use fixed-width, big-endian wire representations.
template <class Integer>
void AppendInteger(std::vector<std::uint8_t>& output, Integer value) {
    using Unsigned = std::make_unsigned_t<Integer>;
    const auto bits = static_cast<Unsigned>(value);
    for (std::size_t index = 0; index < sizeof(Integer); ++index) {
        const auto shift = (sizeof(Integer) - index - 1) * 8;
        output.push_back(static_cast<std::uint8_t>(bits >> shift));
    }
}

// Leave both the cursor and destination unchanged when input is incomplete.
template <class Integer>
bool ReadInteger(std::span<const std::uint8_t> bytes, std::size_t& offset, Integer& value) {
    if (offset > bytes.size() || bytes.size() - offset < sizeof(Integer)) {
        return false;
    }
    using Unsigned = std::make_unsigned_t<Integer>;
    Unsigned bits = 0;
    for (std::size_t index = 0; index < sizeof(Integer); ++index) {
        bits = static_cast<Unsigned>((bits << 8U) | bytes[offset++]);
    }
    value = static_cast<Integer>(bits);
    return true;
}

}  // namespace bre::binary
