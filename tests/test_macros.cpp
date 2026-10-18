#include <limits>
#include <pans/macros.h>
#include <type_traits>

static_assert(sizeof(u8) == 1 && std::is_unsigned_v<u8>);
static_assert(sizeof(u16) == 2 && std::is_unsigned_v<u16>);
static_assert(sizeof(u32) == 4 && std::is_unsigned_v<u32>);
static_assert(sizeof(u64) == 8 && std::is_unsigned_v<u64>);
static_assert(sizeof(i8) == 1 && std::is_signed_v<i8>);
static_assert(sizeof(i16) == 2 && std::is_signed_v<i16>);
static_assert(sizeof(i32) == 4 && std::is_signed_v<i32>);
static_assert(sizeof(i64) == 8 && std::is_signed_v<i64>);

static void test_assert_macros() {
    ASSERT_NOEFFECT2(true, "这个断言不会被触发");
    for (int i = 0; i < 10; ++i) {
        constexpr int NEVER_REACHED_CONDITION = 5;
        ASSERT_CONTINUE2(i != NEVER_REACHED_CONDITION, "fuck");
        std::cout << i << "\n";
    }
}

int main() {
#ifdef PANS_DEBUG
    std::cout << "debug mode, assert will terminate the program." << std::endl;
#endif
#ifdef NDEBUG
    std::cout << "release mode" << std::endl;
#endif

    test_assert_macros();
    return 0;
}
