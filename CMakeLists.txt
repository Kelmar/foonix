cmake_minimum_required(VERSION 3.25)

# =========================================================================

# Read user's selected configuration file
include("${CMAKE_SOURCE_DIR}/src/cmake/config.cmake")

#include("${CMAKE_SOURCE_DIR}/src/cmake/_tools.cmake")
include("${CMAKE_SOURCE_DIR}/src/cmake/target.cmake")

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_ASM_COMPILER clang++)

# =========================================================================

set(CMAKE_CXX_STANDARD 23)

# We don't want to install to our host OS!
set(SYSROOT "${CMAKE_BINARY_DIR}/sysroot")
set(CMAKE_INSTALL_PREFIX ${SYSROOT})

# We also don't want includes from our host OS!
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${SYSROOT} -nostdinc -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${SYSROOT} -nostdinc -nostdinc++ -nostdlib")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --sysroot=${SYSROOT} -nostdinc -nostdinc++ -nostdlib")

# Except we DO want the compiler includes.
execute_process(
    COMMAND clang -print-resource-dir
    OUTPUT_VARIABLE CLANG_RESOURCE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

project(foonix C CXX ASM)

include_directories(
    SYSTEM
    "${CLANG_RESOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src/sys/include"
)

add_custom_command(
    OUTPUT sysroot
    PRE_BUILD
    COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/sysroot_init.sh
    COMMENT "Generating system file structure"
)

add_custom_target(foonix ALL DEPENDS sysroot)

add_subdirectory(src/sys)
add_subdirectory(src/grub)

add_dependencies(foonix kernel)

#install(
#    CODE "execute_process(COMMAND grub-mkrescue -o boot.iso sysroot)"
#)

set_property(
    TARGET foonix
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES boot.iso
)

# =========================================================================
