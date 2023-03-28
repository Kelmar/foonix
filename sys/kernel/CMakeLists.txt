cmake_minimum_required(VERSION 3.20)

project(kernel)

#[===[
link_directories(
    ../tools/local/lib
    ../libk
)
#]===]

include_directories(
    ${CMAKE_SOURCE_DIR}/sys/include
)

add_executable(kernel main.cpp)
add_dependencies(kernel k)

target_compile_options(kernel PRIVATE
    -Wall -Wextra -Wpedantic -Werror -ffreestanding -m32
)

target_link_options(kernel PRIVATE -nostdlib)
target_link_libraries(kernel PRIVATE k)
