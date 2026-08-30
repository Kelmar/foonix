cmake_minimum_required(VERSION 3.20)

# =========================================================================

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (FOO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set (GLOBAL_INCLUDE_DIR "${FOO_ROOT_DIR}/include")

project (foonix_tests)

# =========================================================================

#set(SANITIZE_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)

# =========================================================================

enable_testing()

add_subdirectory("sys/libs/kstubs")
add_subdirectory("sys/libs/libk")
add_subdirectory("sys/libs/libkpp")

# =========================================================================
