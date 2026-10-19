#include <chrono>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <string>

void test_time(std::size_t times) {
    const std::string &format = "%Y-%m-%d %H:%M:%S";
    static thread_local time_t s_lastSecond = 0;
    static thread_local char s_cachedDateTime[20] = {'\0'};
    auto t1 = std::chrono::steady_clock::now();
    // 计算空转时间(仅未开启优化时有效)
    for (std::size_t i = 0; i < times; ++i) {
    }

    auto t2 = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < times; ++i) {
        struct tm tm;
        time_t now = time(0);
        localtime_r(&now, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), format.c_str(), &tm);
    }

    auto t3 = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < times; ++i) {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        time_t current_second =
            std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        // long micros = std::chrono::duration_cast<std::chrono::microseconds>(
        //                   duration % std::chrono::seconds(1))
        //                   .count();
        // localtime_r和strftime的消耗是system_clock的十几倍
        // 所以秒数产生变化时,才更新字符串
        if (current_second != s_lastSecond) {
            struct tm buf;
            localtime_r(&current_second, &buf);
            strftime(s_cachedDateTime, sizeof(s_cachedDateTime), format.c_str(),
                     &buf);
            s_lastSecond = current_second;
        }
    }
    auto t4 = std::chrono::steady_clock::now();
    auto base = t2 - t1;
    auto diff1 = t3 - t2 - base;
    auto diff2 = t4 - t3 - base;

    auto base_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(base).count();
    auto diff1_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(diff1).count();
    auto diff2_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(diff2).count();

    std::cout << "基础耗时: " << base_ns << "ns" << std::endl;
    std::cout << "原耗时: " << diff1_ns << "ns" << std::endl;
    std::cout << "现耗时: " << diff2_ns << "ns" << std::endl;
    std::cout << "优化倍数: " << double(diff1_ns) / diff2_ns << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "need times" << std::endl;
        return -1;
    }
    std::size_t times = std::stoll(argv[1]);
    if (times <= 0) {
        std::cerr << "invaild argument" << std::endl;
        return -1;
    }
    test_time(times);
    return 0;
}
