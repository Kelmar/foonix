cmake_minimum_required(VERSION 3.20)

project(libc)

link_directories(
    ../tools/local/lib
    ../sysroot/usr/lib
)

include_directories(
    ${CMAKE_SOURCE_DIR}/sys/include
)

file(GLOB stdio_srcs stdio/*.c)
file(GLOB stdlib_srcs stdlib/*.c)
file(GLOB string_srcs string/*.c)

add_definitions(-D__is_libk)
add_library(
    libk STATIC
    ${stdio_srcs}
    ${stdlib_srcs}
    ${string_srcs}
    cppfill.cpp
)

target_compile_options(libk PRIVATE
    -Wall -Wextra -Wpedantic -Werror -ffreestanding -m32
)

target_link_options(libk PRIVATE -nostdlib -nostdlib++)
target_link_libraries(libk PRIVATE gcc)
