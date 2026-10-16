cmake_minimum_required(VERSION 3.31)

# 强制设置c/cpp编译器
set(CMAKE_C_COMPILER "/usr/bin/gcc" CACHE FILEPATH "C Compiler" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE FILEPATH "C++ Compiler" FORCE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(pans VERSION 0.1.0 LANGUAGES CXX C)


include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(PANS_BUILD_EXAMPLES "Build examples" ON)
option(PANS_BUILD_TESTS    "Build tests"    ON)
option(PANS_BUILD_TOOLS    "Build tools"    ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# 给 CMAKE_BUILD_TYPE 指定可选值, 代表构建类型，并在 CMake 配置时打印当前构建类型。
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


# 自动计算最适合的编译线程数
cmake_host_system_information(RESULT CPU_NUMS QUERY NUMBER_OF_LOGICAL_CORES)
message(STATUS "CPU NUMS: ${CPU_NUMS}")
math(EXPR JOBS "1 * ${CPU_NUMS}")
set(CMAKE_BUILD_PARALLEL_LEVEL ${JOBS} CACHE STRING "并行编译线程数")
message(STATUS "Compiler default work jobs: ${CMAKE_BUILD_PARALLEL_LEVEL}")

include(cmake/pansOptions.cmake)

add_subdirectory(src)
if (PANS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()
if (PANS_BUILD_TOOLS)
    add_subdirectory(tools)
endif ()




