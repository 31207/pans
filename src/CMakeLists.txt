# 递归收集当前目录下的原文件,做成文件列表
# CONFIGURE_DEPENDS代表有更改时自动更新生成
file(GLOB_RECURSE PANS_SOURCES CONFIGURE_DEPENDS *.cpp *.cc *.c)
file(GLOB_RECURSE PANS_HEADERS CONFIGURE_DEPENDS *.hpp *.h)

add_library(pans STATIC ${PANS_SOURCES} ${PANS_HEADERS}
        ../tests/test_macros.cpp)

target_include_directories(pans
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_link_libraries(pans PUBLIC pans_options)

target_compile_options(pans PRIVATE
    # 主库可能需要更严格的警告
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -Werror
        >
)

install(TARGETS pans
    EXPORT pansTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# 这条命令把 ${PANS_HEADERS} 变量中列出的文件安装到指定目录。
install(FILES ${PANS_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pans
)

