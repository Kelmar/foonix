cmake_minimum_required(VERSION 3.20)

project(libc)

SET(GCC_COVERAGE_LINK_FLAGS "-nostdlib -lgcc")

link_directories(
    ../tools/local/lib
    ../sysroot/usr/lib
)

include_directories(
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/tools/local/include
    ${CMAKE_SOURCE_DIW}/sysroot/usr/include
    include
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
)

target_compile_options(libk PRIVATE
    -Wall -Wextra -Wpedantic -Werror -ffreestanding -m32
)


#[===[

remove_definitions(-D__is_libk)
add_definitions(-D__is_libc)
add_library(
    libc SHARED
    ${stdio_srcs}
    ${stdlib_srcs}
    ${string_srcs}
)

#]===]
