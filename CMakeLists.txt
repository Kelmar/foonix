cmake_minimum_required(VERSION 3.25)

# Must be included first
include("${CMAKE_SOURCE_DIR}/cmake/_tools.cmake")

include("${CMAKE_SOURCE_DIR}/cmake/target.cmake")


set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/config/${CONFIG}.json")

if (NOT EXISTS ${CONFIG_FILE})
  message(FATAL_ERROR "Unknown configuration: ${CONFIG}")
endif()

file(READ "${CONFIG_FILE}" CONFIG_JSON)

string(JSON CONFIG_NAME GET "${CONFIG_JSON}" name)
message("Building configuration: ${CONFIG_NAME}")

string(JSON IS_LINT ERROR_VARIABLE _ GET "${CONFIG_JSON}" isLint)

#if (IS_LINT)
#  message(FATAL_ERROR "Not building a lint configuraiton.  Remove isLint or set to false.")
#endif()

string(JSON USE_BOCHS ERROR_VARIABLE _ GET "${CONFIG_JSON}" useBochs)
string(JSON PLATFORM ERROR_VARIABLE _ GET "${CONFIG_JSON}" platform)

if (PLATFORM)
  message(STATUS "Building for host: ${PLATFORM}")
  set(TARGET_HOST "${PLATFORM}")
else()
  get_default_target()

  message(STATUS "Platform not set defaulting to: ${TARGET_HOST}")
  set(PLATFORM "${TARGET_HOST}")
endif()


set(TOOLS_LOCAL ${CMAKE_SOURCE_DIR}/tools/local)
set(TOOLS_BIN ${TOOLS_LOCAL}/bin)

set(SYSROOT ${CMAKE_BINARY_DIR}/sysroot)


if (${USE_BOCHS})
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
