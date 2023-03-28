cmake_minimum_required(VERSION 3.20)

project(kernel C CXX ASM)

#[===[
link_directories(
    ../tools/local/lib
    ../libk
)
#]===]

set(HOST_DIR ${CMAKE_SOURCE_DIR}/sys/kernel/arch/${TARGET_HOST})

include_directories(
    ${CMAKE_SOURCE_DIR}/sys/include
    ${HOST_DIR}/include
)

list(APPEND SOURCES main.cpp)

include(${HOST_DIR}/build.cmake)

add_executable(kernel ${SOURCES})
add_dependencies(kernel k)

target_compile_options(kernel PRIVATE
    -Wall -Wextra -Wpedantic -Werror -ffreestanding -fno-rtti -fno-exceptions -m32 -g
)

#SET(GCC_COVERAGE_LINK_FLAGS "-T")

target_link_options(kernel PRIVATE -nostdlib -T${CMAKE_SOURCE_DIR}/sys/kernel/arch/${TARGET_HOST}/linker.ld)
target_link_libraries(kernel PRIVATE k)

install(
    TARGETS kernel RUNTIME
    DESTINATION ${SYSROOT}/boot
)
