cmake_minimum_required(VERSION 3.20)

project(libc C CXX ASM)

link_directories(
    ../tools/local/lib
    ../sysroot/usr/lib
)

file(GLOB stdio_srcs stdio/*.c)
file(GLOB string_srcs string/*.c)

include(arch/${TARGET_HOST}/build.cmake)

add_definitions(-D__is_libk)
add_library(
    k STATIC
    ${stdio_srcs}
    ${string_srcs}
    cppfill.cpp
)

target_compile_options(k PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>: -fno-rtti -fno-exceptions> -ggdb
)

target_link_options(k PRIVATE -nostdlib)
target_link_libraries(k PRIVATE gcc)
target_include_directories(k PUBLIC include)
