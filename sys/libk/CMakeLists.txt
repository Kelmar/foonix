cmake_minimum_required(VERSION 3.20)

project(libc C CXX ASM)

link_directories(
    ../tools/local/lib
    ../sysroot/usr/lib
)

include_directories(
    ${CMAKE_SOURCE_DIR}/sys/include
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

# When you tell Clang "Pedantic" it means it!

target_compile_options(k PRIVATE
    -Wall -Wextra -Wpedantic -Wno-unused-command-line-argument -Werror -ffreestanding -m32 -g
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>: -fno-rtti -fno-exceptions>
)

target_link_options(k PRIVATE -nostdlib)
target_link_libraries(k PRIVATE gcc)
target_include_directories(k PUBLIC include)
