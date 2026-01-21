set(CMAKE_SYSTEM_NAME Generic-ELF)
set(CMAKE_CROSSCOMPILING TRUE)

set(TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools")
set(TOOLS_BIN_DIR "${TOOLS_DIR}/bin")
set(TOOLS_PREFIX "${TOOLS_BIN_DIR}/${PLATFORM}-elf-")

set(CMAKE_C_COMPILER ${TOOLS_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLS_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${TOOLS_PREFIX}g++)

# Tell CMake to skip the compiler checks.
# The normal checks fail because of missing build pieces.
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_ASM_COMPILER_WORKS 1)

# We will however verify that the files are were we expect them to be
if (NOT EXISTS "${CMAKE_C_COMPILER}")
    message(FATAL_ERROR "Cross copmiler not built, please run build.sh in the build directory first.")
endif()

if (NOT EXISTS "${CMAKE_CXX_COMPILER}")
    message(FATAL_ERROR "Cross copmiler not built, please run build.sh in the build directory first.")
endif()

if (NOT EXISTS "${CMAKE_ASM_COMPILER}")
    message(FATAL_ERROR "Cross copmiler not built, please run build.sh in the build directory first.")
endif()

# =========================================================================

# Trackdown where the compiler includes are.
# These are things like stddef.h and stdint.h, which we want/need
execute_process(
    COMMAND sh -c "${CMAKE_CXX_COMPILER} --version | head -n 1 | awk '{ print $NF; }'"
    OUTPUT_VARIABLE CXX_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(STRIP ${CXX_VERSION} CXX_VERSION)

set(COMP_DIR "${TOOLS_DIR}/lib/gcc/${PLATFORM}-elf/${CXX_VERSION}")

if ( NOT EXISTS "${COMP_DIR}/include/stdint.h" )
    message(FATAL_ERROR "Couldn't find ${COMP_DIR}/include/stdint.h")
endif()

include_directories(SYSTEM "${COMP_DIR}/include")

unset(COMP_DIR)

# =========================================================================

#set(W_FLAGS "-Wall -Wextra -Wpedantic -Wno-unused-command-line-argument -Werror")
set(W_FLAGS "-Wall -Wextra -Wpedantic -Werror")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${W_FLAGS} -ffreestanding -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${W_FLAGS} -ffreestanding -nostdlib")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${W_FLAGS} -ffreestanding -nostdlib")
