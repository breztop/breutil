#include <cassert>
#include <cstdint>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "breutil/encoding/base64.hpp"

namespace {

void check_vectors() {
    constexpr std::pair<std::string_view, std::string_view> vectors[] = {
        {"", ""},       {"f", "Zg=="},     {"fo", "Zm8="},
        {"foo", "Zm9v"}, {"foob", "Zm9vYg=="}, {"fooba", "Zm9vYmE="},
        {"foobar", "Zm9vYmFy"},
    };
    for (const auto& [plain, encoded] : vectors) {
        assert(bre::Base64::Encode(plain) == encoded);
        std::string decoded;
        assert(bre::Base64::Decode(encoded, decoded));
        assert(decoded == plain);
    }
}

void check_invalid_input() {
    constexpr std::string_view invalid[] = {
        "A", "AAA", "====", "A===", "AA=A", "=AAA", "AAA!", "Zh==", "Zm9=",
    };
    for (const auto input : invalid) {
        std::string output = "must be cleared";
        assert(!bre::Base64::IsBase64Encoded(input));
        assert(!bre::Base64::Decode(input, output));
        assert(output.empty());
    }
}

void check_random_round_trips() {
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> bytes(0, 255);
    for (std::size_t size = 0; size <= 4097; size += 17) {
        std::string input(size, '\0');
        for (char& value : input) value = static_cast<char>(bytes(generator));

        const std::string encoded = bre::Base64::Encode(input);
        assert(bre::Base64::IsBase64Encoded(encoded));

        std::vector<std::uint8_t> decoded;
        assert(bre::Base64::DecodeFromArray(encoded.data(), encoded.size(), decoded));
        assert(decoded.size() == input.size());
        for (std::size_t i = 0; i < input.size(); ++i)
            assert(decoded[i] == static_cast<std::uint8_t>(input[i]));
    }
}

}  // namespace

int main() {
    check_vectors();
    check_invalid_input();
    check_random_round_trips();

    std::string output = "not empty";
    assert(!bre::Base64::DecodeFromArray(nullptr, 1, output));
    assert(output.empty());
}
