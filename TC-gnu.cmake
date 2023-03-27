set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER   i686-elf-gcc)
set(CMAKE_CXX_COMPILER i686-elf-g++)

# Set the target environment to our sysroot build folder
set(CMAKE_FIND_ROOT_PATH sysroot)

# Search for programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
