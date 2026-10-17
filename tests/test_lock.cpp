#include <iostream>
#include <mutex>
#include <atomic>
#include <iomanip>
#include <vector>
#include <shared_mutex>
#include <thread>
#include <latch>
constexpr int RUNS = 7;

static uint64_t g_target = 0;
static uint64_t g_counter = 0;
static std::atomic<uint64_t> g_atomicCounter(0);
static std::mutex g_mutex;
static std::shared_mutex g_sharedMutex;

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

std::chrono::nanoseconds BenchmarkMutex(std::size_t thread_count) {
    g_counter = 0;
    const auto ELAPSED = BenchmarkThreads(thread_count, [](std::size_t, uint64_t operations) {
        for (uint64_t i = 0; i < operations; ++i) {
            std::lock_guard<std::mutex> lock(g_mutex);
            ++g_counter;
        }
    });
    return ELAPSED;
}

std::chrono::nanoseconds BenchmarkSharedMutex(std::size_t thread_count) {
    g_counter = 0;
    const auto ELAPSED = BenchmarkThreads(thread_count, [](std::size_t, uint64_t operations) {
        for (uint64_t i = 0; i < operations; ++i) {
            std::unique_lock<std::shared_mutex> lock(g_sharedMutex);
            ++g_counter;
        }
    });
    return ELAPSED;
}

std::chrono::nanoseconds BenchmarkAtomic(std::size_t thread_count) {
    // g_atomicCounter = 0;
    g_atomicCounter.store(0, std::memory_order_relaxed);
    const auto ELAPSED = BenchmarkThreads(thread_count, [](std::size_t, uint64_t operations) {
        for (uint64_t i = 0; i < operations; ++i) {
            // ++g_atomicCounter;
            g_atomicCounter.fetch_add(1, std::memory_order_relaxed);
        }
    });
    return ELAPSED;
}

// CAS（Compare-And-Swap，比较并交换）
std::chrono::nanoseconds BenchmarkAtomicCas(std::size_t thread_count) {
    g_atomicCounter.store(0, std::memory_order_relaxed);
    const auto ELAPSED = BenchmarkThreads(thread_count, [](std::size_t, uint64_t operations) {
        for (uint64_t i = 0; i < operations; ++i) {
            uint64_t expected = g_atomicCounter.load(std::memory_order_relaxed);
            while (!g_atomicCounter.compare_exchange_weak(expected, expected + 1, std::memory_order_relaxed)) {
            }
        }
    });
    return ELAPSED;
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <target>" << std::endl;
        return 1;
    }
    g_target = std::stoi(argv[1]);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "operations: " << g_target << " average of " << RUNS << " runs" << std::endl;
    const std::vector<std::size_t> THREAD_COUNTS = {1, 2, 4, 8, 10};
    for (auto &thread_count: THREAD_COUNTS) {
        std::cout << "threads: " << thread_count << std::endl;
        PrintResult("mutex", Run([&]() { return BenchmarkMutex(thread_count); }));
        PrintResult("shared mutex", Run([&]() { return BenchmarkSharedMutex(thread_count); }));
        PrintResult("atomic", Run([&]() { return BenchmarkAtomic(thread_count); }));
        PrintResult("atomic CAS", Run([&]() { return BenchmarkAtomicCas(thread_count); }));
        std::cout << std::endl;
    }
}
