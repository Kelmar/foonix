set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER   ${TOOLS_BIN}/i686-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLS_BIN}/i686-elf-g++)

#[===[
CMake is going to complain about our compiler not working.

This is because there is no crt0 to link to for basic programs
and we need very specific flags to get the libraries to compile
correctly.

The options below force it to accept our "broken" compiler.
#]===]

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Set the target environment to our sysroot build folder
set(CMAKE_FIND_ROOT_PATH ${SYSROOT})

# Search for programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(TARGET_HOST i386)