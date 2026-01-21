cmake_minimum_required(VERSION 3.25)

# =========================================================================

# Read user's selected configuration file
include("${CMAKE_SOURCE_DIR}/cmake/config.cmake")

include("${CMAKE_SOURCE_DIR}/cmake/_tools.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/target.cmake")

# =========================================================================

# We don't want to install to our host OS!
set(SYSROOT "${CMAKE_BINARY_DIR}/sysroot")
set(CMAKE_INSTALL_PREFIX ${SYSROOT})

# We also don't want includes from our host OS!
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${SYSROOT} -nostdinc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${SYSROOT} -nostdinc -nostdinc++")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --sysroot=${SYSROOT} -nostdinc -nostdinc++")

project(foonix C CXX ASM)

include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/sys/include")

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
