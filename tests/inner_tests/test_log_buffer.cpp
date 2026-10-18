#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <iostream>
#include <logger/buffer.h>
#include <logger/buffer_config.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
constexpr int RUNS = 7;
std::uint64_t g_target = 0;
std::atomic<std::uint64_t> BENCHMARK_SINK{0};

std::uint64_t Observe(std::string_view value) noexcept {
    return static_cast<std::uint64_t>(value.size()) +
           static_cast<unsigned char>(value.front()) +
           static_cast<unsigned char>(value[value.size() / 2]) +
           static_cast<unsigned char>(value.back());
} // ???

std::uint64_t WriteWithSmallStreamBuffer(std::string_view message) {
    pans::detail::InlineBuffer<pans::detail::LOG_MESSAGE_INLINE_CAPACITY>
        buffer;
    pans::detail::SmallStringBuffer<pans::detail::LOG_MESSAGE_INLINE_CAPACITY>
        stream_buffer(buffer);
    std::ostream stream(&stream_buffer);
    stream << message;
    return Observe(buffer.view());
}

std::uint64_t WriteWithStringStream(std::string_view message) {
    std::stringstream stream;
    stream << message;
    return Observe(stream.view());
}

template <typename Operation>
std::chrono::nanoseconds BenchmarkWrites(Operation &&op) {
    std::uint64_t check_sum = 0;
    const auto BEGIN = std::chrono::steady_clock::now();
    for (std::uint64_t i = 0; i < g_target; ++i) {
        check_sum += op();
    }
    const auto END = std::chrono::steady_clock::now();
    BENCHMARK_SINK.fetch_xor(check_sum, std::memory_order_relaxed);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(END - BEGIN);
}

void PrintResult(std::string_view name, double total_nanoseconds) {
    const double NANOSECONDS_PER_OPERATION =
        total_nanoseconds / static_cast<double>(g_target);
    std::cout << std::left << std::setw(14) << name << std::right
              << total_nanoseconds << "ns \ttotal, "
              << NANOSECONDS_PER_OPERATION << "ns per operation" << std::endl;
}
template <typename Benchmark> double Run(Benchmark &&benchmark) {
    long double total_nanosecond = 0.0L;
    for (int i = 0; i < RUNS; ++i) {
        total_nanosecond += static_cast<long double>(benchmark().count());
    }
    return static_cast<double>(total_nanosecond / RUNS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "count!" << std::endl;
        return 1;
    }
    g_target = std::stoull(argv[1]);
    std::cout << std::fixed << std::setprecision(2);

    std::cout << "operations: " << g_target << ", average of " << RUNS
              << " runs" << std::endl;
    constexpr std::array<std::size_t, 6> MESSAGE_SIZE = {50,  100,  200,
                                                         500, 1000, 2048};
    for (auto &message_size : MESSAGE_SIZE) {
        const std::string message(message_size, 'x');
        const double SMALL_STREAM_BUFFER_NANOSECONDS = Run([&]() {
            return BenchmarkWrites(
                [&]() { return WriteWithSmallStreamBuffer(message); });
        });
        const double STRINGSTREAM_NANOSECONDS = Run([&]() {
            return BenchmarkWrites(
                [&]() { return WriteWithStringStream(message); });
        });
        std::cout << "msg_size:" << message_size << std::endl;
        PrintResult("SmallStreamBuffer", SMALL_STREAM_BUFFER_NANOSECONDS);
        PrintResult("std::stringstream", STRINGSTREAM_NANOSECONDS);
        std::cout << "speedup: "
                  << STRINGSTREAM_NANOSECONDS / SMALL_STREAM_BUFFER_NANOSECONDS
                  << std::endl;
    }
    return 0;
}
