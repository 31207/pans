#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>
#include <latch>
#include <thread>
#include <pans/mutex.h>
constexpr int RUNS = 7;
std::uint64_t g_counter = 0;
std::uint64_t g_target = 0;


uint64_t OperationsForThread(std::size_t thread_index, std::size_t thread_count) {
    const uint64_t BASE = g_target / thread_count;
    const uint64_t REMAINDER = g_target % thread_count;
    return BASE + (thread_index < REMAINDER ? 1 : 0);
}


// 使用thread_count个线程, 跑operation这个操作
template<typename Operation>
std::chrono::nanoseconds BenchmarkThreads(std::size_t thread_count, Operation &&operation) {
    // 其实ptrdiff_t就是个有符号整数, 直接给个有符号整数完全没问题, 不是很能理解这玩意啥作用
    std::latch ready(static_cast<std::ptrdiff_t>(thread_count));
    std::latch start_gate(1);
    std::latch finished(static_cast<std::ptrdiff_t>(thread_count));

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    for (std::size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back([&, i]() {
            ready.count_down(); // 每启动一个线程, ready值-1
            start_gate.wait(); // 等待该信号量降至0
            operation(i, OperationsForThread(i, thread_count));
            finished.count_down();
        });
    }
    ready.wait(); // 确保所有工作线程都已经准备就绪，再开始计时
    const auto BEGIN = std::chrono::steady_clock::now();
    start_gate.count_down();
    finished.wait();
    const auto END = std::chrono::steady_clock::now();
    for (auto &thread: threads) {
        thread.join();
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(END - BEGIN);
}


template<typename T>
std::chrono::nanoseconds BenchmarkMutex(std::uint64_t thread_count) noexcept {
    T mutex;
    g_counter = 0;
    const auto elapsed = BenchmarkThreads(thread_count, [&mutex](std::size_t, std::uint64_t operations) {
        for (std::uint64_t i = 0; i < operations; ++i) {
            std::lock_guard<T> lock(mutex);
            ++g_counter;
        }
    });
    if (g_counter != g_target) {
        throw std::runtime_error("BenchmarkMutex: thread count mismatch");
    }
    return elapsed;
}

// 跑指定次数的函数, 取平均执行时间
template<typename T>
double Run(T &&benchmark) {
    long double total_nanoseconds = 0;
    for (int run = 0; run < RUNS; ++run) {
        total_nanoseconds += static_cast<long double>(benchmark().count());
    }
    return static_cast<double>(total_nanoseconds / RUNS);
}

void PrintResult(std::string_view name, double total_nanoseconds) {
    const double NANOSECONDS_PER_OPERATION = total_nanoseconds / static_cast<double>(g_target);
    std::cout << std::left << std::setw(14) << name << std::right
            << total_nanoseconds << "ns \ttotal, "
            << NANOSECONDS_PER_OPERATION << "ns per operation" << std::endl;
}

class SpinLock2 {
public:
    SpinLock2() noexcept = default;

    ~SpinLock2() noexcept = default;

    SpinLock2(const SpinLock2 &) = delete;

    SpinLock2 &operator=(const SpinLock2 &) = delete;

    void lock() noexcept {
        while (m_mutex.test_and_set(std::memory_order_acquire)) {
            while (m_mutex.test(std::memory_order_relaxed)) {
                cpu_relax();
            }
        }
    }

    [[nodiscard]] bool try_lock() noexcept {
        return !m_mutex.test_and_set(std::memory_order_acquire);
    }

    void unlock() noexcept {
        m_mutex.clear(std::memory_order_release);
    }

private:
    std::atomic_flag m_mutex = ATOMIC_FLAG_INIT;
};


int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <target>" << std::endl;
        return EXIT_FAILURE;
    }
    g_target = std::stoull(argv[1]);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "operations: " << g_target << ", average of " << RUNS << " runs" << std::endl;
    const std::vector<std::size_t> thread_counts = {1, 2, 4, 8, 10, 20, 32, 48, 72};
    // 当线程数开始大于cpu核心数量的时候, std::mutex的优势会逐渐显现出来, 自旋锁会越来越慢
    for (auto &thread_count: thread_counts) {
        std::cout << "threads: " << thread_count << std::endl;
        PrintResult("pthread_spinlock: ", Run([&]() { return BenchmarkMutex<pans::SpinLock>(thread_count); }));
        PrintResult("atomic_flag spinlock ", Run([&]() { return BenchmarkMutex<SpinLock2>(thread_count); }));
        PrintResult("std::mutex: ", Run([&]() { return BenchmarkMutex<std::mutex>(thread_count); }));
        std::cout << std::endl;
    }
}
