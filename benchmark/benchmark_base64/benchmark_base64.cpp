#include <benchmark/benchmark.h>

#include <cstdint>
#include <random>
#include <string>

#include "breutil/encoding/base64.hpp"

namespace {

std::string make_input(std::size_t size) {
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> bytes(0, 255);
    std::string input(size, '\0');
    for (char& value : input) {
        value = static_cast<char>(bytes(generator));
    }
    return input;
}

void BM_Base64Validate(benchmark::State& state) {
    const std::string encoded = bre::Base64::Encode(make_input(state.range(0)));
    for (auto _ : state) {
        bool valid = bre::Base64::IsBase64Encoded(encoded);
        benchmark::DoNotOptimize(std::move(valid));
    }
    state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(encoded.size()));
}
BENCHMARK(BM_Base64Validate)->RangeMultiplier(64)->Range(1024, 1 << 24);

void BM_Base64Decode(benchmark::State& state) {
    const std::string encoded = bre::Base64::Encode(make_input(state.range(0)));
    std::string decoded;
    for (auto _ : state) {
        bool success = bre::Base64::Decode(encoded, decoded);
        benchmark::DoNotOptimize(std::move(success));
        benchmark::ClobberMemory();
    }
    state.SetBytesProcessed(state.iterations() * static_cast<std::int64_t>(encoded.size()));
}
BENCHMARK(BM_Base64Decode)->RangeMultiplier(16)->Range(64, 1 << 20);

}  // namespace

BENCHMARK_MAIN();
