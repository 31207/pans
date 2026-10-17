#ifndef INCLUDE_PANS_MACROS_H
#define INCLUDE_PANS_MACROS_H

#include <iostream>
#include <cassert>
#include <cstdint>

#define PANS_ASSERT(x) \
    if(!(x)) [[unlikely]] { \
        std::cerr << __FILE__ << ":" << __LINE__ << "ASSERT FAILED: " << #x << "\n Stacktrace: TODO" << "\n"; \
        assert(x); \
    }
#define PANS_ASSERT2(x, info) \
    if(!(x)) [[unlikely]] { \
        std::cerr << __FILE__ << ":" << __LINE__ << "ASSERT FAILED: " << #x << "\ninfo:" << info << "\nStacktrace: TODO" << "\n"; \
        assert(x); \
    }

#define ASSERT_RETVAL(x, val) \
    do {\
        if(x) [[likely]] break;\
        PANS_ASSERT(x);\
        return val;\
    }while(0);
#define ASSERT_RETVAL2(x, val, info) \
    do {\
        if(x) [[likely]] break;\
        PANS_ASSERT2(x, info);\
        return val;\
    }while(0);
#define ASSERT_RETNONE(x) \
    do {\
        if(x) [[likely]] break;\
        PANS_ASSERT(x);\
        return;\
    }while(0);

#define ASSERT_RETNONE2(x, info) \
    do {\
        if(x) [[likely]] break;\
        PANS_ASSERT2(x, info);\
        return;\
    }while(0);
#define ASSERT_NOEFFECT(x) \
    do {\
        if(x) [[likely]] break;\
        PANS_ASSERT(x);\
    }while(0);

#define ASSERT_NOEFFECT2(x, info) \
    do {\
        if(x) [[likely]] break;\
        PANS_ASSERT2(x, info);\
    }while(0);
#define ASSERT_CONTINUE(x) \
        if(!(x)) [[unlikely]] {\
        PANS_ASSERT(x);\
        continue;\
        }else{}
#define ASSERT_CONTINUE2(x, info) \
        if(!(x)) [[unlikely]] {\
        PANS_ASSERT2(x, info);\
        continue;\
        }else{}
#define ASSERT_BREAK(x) \
        if(!(x)) [[unlikely]] {\
        PANS_ASSERT(x);\
        break;\
        }else{}
#define ASSERT_BREAK2(x, info) \
        if(!(x)) [[unlikely]] {\
        PANS_ASSERT2(x, info);\
        break;\
        }else{}

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

#endif
