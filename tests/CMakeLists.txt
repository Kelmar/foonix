cmake_minimum_required(VERSION 3.20)
project(foonix_libk_tests C)

# =========================================================================
# Standalone project: builds and runs libk against the HOST compiler, to
# validate its behavior without needing to boot a kernel.
#
# Deliberately NOT wired into the top-level foonix CMake tree via
# add_subdirectory() -- that tree locks in the i686-elf/x86_64-elf
# cross-compiler and -ffreestanding/-nostdlib flags before project() is
# even called (see ../cmake/_tools.cmake), and CMake can't mix compilers
# within a single configure. This is a second, independent build:
#
#   cmake -S tests -B tests/build
#   cmake --build tests/build
#   ctest --test-dir tests/build --output-on-failure
# =========================================================================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(FOONIX_TESTS_SANITIZE "Build with AddressSanitizer + UndefinedBehaviorSanitizer" ON)

set(SYS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sys/include")
set(LIBK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sys/libk")

set(SANITIZE_FLAGS "")

if (FOONIX_TESTS_SANITIZE)
    set(SANITIZE_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

# =========================================================================
# The libk sources under test, compiled unmodified against the project's
# own freestanding headers -- the same <string.h>/<stdio.h> the kernel
# build uses, so this exercises exactly what ships.
#
# -fno-builtin matters here: without it, GCC/Clang will happily recognize
# calls like memcpy()/memset() with compile-time-constant sizes and inline
# its own builtin implementation instead of calling ours, which would
# silently stop these tests from testing our code at all.
#
# LIBK_SYMBOL_RENAMES matters for a subtler reason: AddressSanitizer ships
# its OWN interceptors for a fixed list of well-known libc function names
# (memcpy, strncat, vsnprintf, ...) and installs them regardless of who
# else defines a same-named symbol. Confirmed the hard way: a call to our
# strncat() was silently rerouted to ASan's interceptor applying POSIX
# strncat semantics (n = max bytes of src) instead of this project's (size
# = total dest capacity) -- our strncat.c was never actually reached.
# Renaming every such symbol at compile time (both the definitions here
# AND every call site in tests/*.c, applied identically) sidesteps the
# collision entirely: nothing named memcpy/strncat/etc. ever gets linked
# or called, so there's nothing for ASan -- or a hosted libc, with or
# without sanitizers -- to intercept.
# =========================================================================

set(LIBK_SYMBOL_RENAMES
    memcpy=foonix_memcpy
    memmove=foonix_memmove
    memset=foonix_memset
    memchr=foonix_memchr
    memcmp=foonix_memcmp
    strlen=foonix_strlen
    strnlen=foonix_strnlen
    strncpy=foonix_strncpy
    strncat=foonix_strncat
    vsnprintf=foonix_vsnprintf
    snprintf=foonix_snprintf
)

add_library(libk_under_test STATIC
    "${LIBK_DIR}/string/memchr.c"
    "${LIBK_DIR}/string/memcmp.c"
    "${LIBK_DIR}/string/memcpy.c"
    "${LIBK_DIR}/string/memmove.c"
    "${LIBK_DIR}/string/memset.c"
    "${LIBK_DIR}/string/strlen.c"
    "${LIBK_DIR}/string/strnlen.c"
    "${LIBK_DIR}/string/strncat.c"
    "${LIBK_DIR}/string/strncpy.c"
    "${LIBK_DIR}/string/_s_strncat.c"
    "${LIBK_DIR}/string/_s_strncpy.c"
    "${LIBK_DIR}/stdio/vsnprintf.c"
    "${LIBK_DIR}/stdio/snprintf.c"
)

target_include_directories(libk_under_test PRIVATE "${SYS_INCLUDE_DIR}")

target_compile_definitions(libk_under_test PRIVATE ${LIBK_SYMBOL_RENAMES})

target_compile_options(libk_under_test PRIVATE
    -Wall -Wextra -Wpedantic -fno-builtin -g ${SANITIZE_FLAGS}
)

# =========================================================================
# Host-side stand-ins for the parts of the kernel libk normally leans on.
# =========================================================================

# kpanic()/kassert(): the real arch/kernel debug console isn't available
# here, so a violated ASSERT() (e.g. vsnprintf's internal bounds checks)
# reports to stderr and abort()s the test process instead of halting the
# "system".
add_library(kstubs STATIC kstubs.c)
target_compile_options(kstubs PRIVATE -Wall -Wextra ${SANITIZE_FLAGS})

# Deliberately built WITHOUT sys/include on its path: the project's own
# <stdio.h> is a minimal freestanding subset (no fprintf/stderr/FILE), so
# this is the one place that needs the real host <stdio.h>.
add_library(test_io STATIC test_io.c)
target_compile_options(test_io PRIVATE -Wall -Wextra ${SANITIZE_FLAGS})

# =========================================================================

add_executable(libk_tests
    main.c
    harness.c
    test_memory.c
    test_strlen.c
    test_bounded_string.c
    test_vsnprintf.c
)

target_include_directories(libk_tests PRIVATE "${SYS_INCLUDE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(libk_tests PRIVATE ${LIBK_SYMBOL_RENAMES})
target_compile_options(libk_tests PRIVATE -Wall -Wextra -fno-builtin ${SANITIZE_FLAGS})
target_link_options(libk_tests PRIVATE ${SANITIZE_FLAGS})
target_link_libraries(libk_tests PRIVATE libk_under_test kstubs test_io)

enable_testing()
add_test(NAME libk_tests COMMAND libk_tests)
