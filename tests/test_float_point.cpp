#include <iostream>

template <typename T>
concept fp_or_int = std::is_floating_point_v<T> || std::is_integral_v<T>;

template <fp_or_int T>
void DisplayBits(T&& num) {
    std::size_t length = sizeof(T) * 8;
    std::uint64_t* real = reinterpret_cast<uint64_t*>(&num);
    std::uint64_t mask = 1;
    for (std::size_t i = 0; i < length; ++i) {
        std::cout << (*real & mask << (length - i - 1)? '1' : '0');
    }
    std::cout << std::endl;
}


int main() {
    DisplayBits(1.1);
    DisplayBits(114514);
    DisplayBits(-1);
    return 0;
}
