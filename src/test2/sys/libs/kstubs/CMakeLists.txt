
project (kstubs C CXX ASM)

add_library(kstubs STATIC kstubs.c main.cpp fwd_printf.c)

target_include_directories(kstubs PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_options(kstubs PRIVATE -Wall -Wextra -Wpedantic -g ${SANTITIZE_FLAGS})
