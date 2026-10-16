#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <pans/macros.h>

#include "../src/include/pans/macros.h"
// 对于复杂类型, 使用mutex搭配lock_guard或unique_lock来上锁
// 简单类型(1,2,4,8字节类型), 使用atomic会快很多, 编译器会生成CPU原子指令.


int num = 0;
std::atomic<int> num2(0);
std::mutex m;
constexpr int THREAD_COUNT = 10;
constexpr int N = 1000000;

void add(int times) {
    for (int i = 0; i < times; ++i) {
        std::unique_lock<std::mutex> lock(m);
        ++num;
        lock.unlock();
        ++num2;
    }
}



int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(std::thread(add, N));
    }
    for (auto& t : threads) t.join();
    std::cout << num << std::endl;
    std::cout << num2 << std::endl;
    PANS_ASSERT2(num == N * THREAD_COUNT, "fuck1!");
    PANS_ASSERT2(num2 == N * THREAD_COUNT, "fuck2!");
    return 0;
}