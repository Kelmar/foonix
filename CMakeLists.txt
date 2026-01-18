cmake_minimum_required(VERSION 3.20)

option(USE_BOCHS "Set if you plan to debug under BOCHS" OFF)

set(TOOLS_LOCAL ${CMAKE_SOURCE_DIR}/tools/local)
set(TOOLS_BIN ${TOOLS_LOCAL}/bin)

set(SYSROOT ${CMAKE_BINARY_DIR}/sysroot)

set(CMAKE_TOOLCHAIN_FILE "TC-foonix.cmake")

if(${USE_BOCHS})
    add_definitions(-D_USE_BOCHS)
endif()

# We don't want to install to our host OS!
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
