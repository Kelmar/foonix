cmake_minimum_required(VERSION 3.20)

set(TOOLS_LOCAL ${CMAKE_SOURCE_DIR}/tools/local)
set(TOOLS_BIN ${TOOLS_LOCAL}/bin)

set(CMAKE_TOOLCHAIN_FILE "TC-foonix.cmake")

project(foonix)

add_custom_command(
    OUTPUT sysroot
    PRE_BUILD
    COMMAND bash scripts/sysroot_init.sh
)

add_subdirectory(sys)

