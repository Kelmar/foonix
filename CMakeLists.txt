cmake_minimum_required(VERSION 3.25)

# =========================================================================

# Must be included first
include("${CMAKE_SOURCE_DIR}/cmake/_tools.cmake")

# =========================================================================

# Read user's selected configuration file
include("${CMAKE_SOURCE_DIR}/cmake/config.cmake")

include("${CMAKE_SOURCE_DIR}/cmake/target.cmake")

# =========================================================================

# We don't want to install to our host OS!
set(SYSROOT ${CMAKE_BINARY_DIR}/sysroot)
set(CMAKE_INSTALL_PREFIX ${SYSROOT})

project(foonix)

add_custom_command(
    OUTPUT sysroot
    PRE_BUILD
    COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/sysroot_init.sh
    COMMENT "Generating system file structure"
)

add_custom_target(foonix ALL DEPENDS sysroot)

add_subdirectory(sys)
add_subdirectory(grub)

add_dependencies(foonix kernel)

install(
    CODE "execute_process(COMMAND grub-mkrescue -o boot.iso sysroot)"
)

# =========================================================================
