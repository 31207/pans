add_library(pans_options INTERFACE)

target_compile_options(pans_options INTERFACE
    # GCC和Clang都通用的编译选项.
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -Wall
        -Wextra    # 额外警告
        -Wpedantic # 严格遵循标准
        -fno-strict-aliasing
    >
)

# 只有目标平台是 Linux，并且 C++ 编译器是 GCC 或 Clang 时，才添加 -fPIC 编译选项。
target_compile_options(pans_options INTERFACE
    $<$<AND:$<PLATFORM_ID:LINUX>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:
        -fPIC # 编译器生成的机器码不依赖于代码最终被加载到内存中的某个固定地址(位置无关代码)
    >
)

# 这次传递的是链接选项
# 条件同上
target_link_options(pans_options INTERFACE
        $<$<AND:$<PLATFORM_ID:LINUX>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:
        -rdynamic # 栈回溯需要
        >
)

target_compile_options(pans_options INTERFACE
    $<$<CONFIG:DEBUG>:-DPANS_DEBUG>
)

# 接下来针对不同编译情况(debug release等)给予不同编译选项
target_compile_options(pans_options INTERFACE
    $<$<CONFIG:Debug>:
        -O0
        -g3 # 生成更完整的调试信息
        -ggdb # 生成适合gdb的调试信息
        >
)

target_compile_options(pans_options INTERFACE
    $<$<CONFIG:Release>:
        -DNDEBUG # 不debug
        -O2
        -fno-omit-frame-pointer
        >
    $<$<CONFIG:RelWithDebInfo>:
        -DNDEBUG
        -O2
        -g
        -fno-omit-frame-pointer
        >
)

# 检查测试代码对源代码的覆盖率
option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(pans_options INTERFACE
        $<$<CONFIG:Debug>:--coverage>
    )
    target_link_options(pans_options INTERFACE
            $<$<CONFIG:Debug>:--coverage>
    )
endif()



