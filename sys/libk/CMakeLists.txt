# =========================================================================
# Kernel library global CMakeLists
# =========================================================================

cmake_minimum_required(VERSION 3.25)

project(libc C CXX ASM)

link_directories(
    ../tools/local/lib
    ../sysroot/usr/lib
)

# =========================================================================
# Find generic implementations

file(GLOB stdio_srcs stdio/*.c)
file(GLOB string_srcs string/*.c)

list(APPEND SOURCES ${stdio_srcs})
list(APPEND SOURCES ${string_srcs})
list(APPEND SOURCES cppfill.cpp)

# =========================================================================
# Find host specific overrides

set(HOST_DIR "${CMAKE_SOURCE_DIR}/sys/libk/arch/${PLATFORM}")

if (EXISTS "${HOST_DIR}/build.cmake")

    include("${HOST_DIR}/build.cmake")

    # Replace generic implementations with platform specific optimizations.

    foreach(opt_file ${platform_files})
        #message("OPT: ${opt_file}")
        get_filename_component(file_name ${opt_file} NAME_WLE)
        set(file_name ".*\\/${file_name}\\.c")
        #message("REPLACE: ${file_name}")
        list(FILTER SOURCES EXCLUDE REGEX "${file_name}")
        list(APPEND SOURCES "${opt_file}")
    endforeach()
endif()

# =========================================================================

add_definitions(-D__is_libk)
add_library(k STATIC ${SOURCES})

target_compile_options(k PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>: -fno-rtti -fno-exceptions> -ggdb
)

target_link_options(k PRIVATE -nostdlib)
#target_link_libraries(k PRIVATE gcc)
target_include_directories(k PUBLIC include)

# =========================================================================
