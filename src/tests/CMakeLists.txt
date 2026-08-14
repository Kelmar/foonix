cmake_minimum_required(VERSION 3.20)

# =========================================================================

project(foonix_libk_tests C)
enable_language(CXX)

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

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Matches the real libk build (sys/libk/CMakeLists.txt: add_definitions(-D__is_libk)), for
# fidelity with what actually ships. Also required for sys/assert.h's ASSERT macro to route
# through kpanic()/kassert() (stubbed in kstubs.c) instead of its non-kernel fprintf()/abort()
# fallback, which assumes a real host <stdio.h>/<stdlib.h> are already visible -- they aren't
# here, since sys/include's own minimal <stdio.h> shadows them (see kstubs.c/test_io.c).
target_compile_definitions(libk_under_test PRIVATE __is_libk ${LIBK_SYMBOL_RENAMES})

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
target_compile_options(kstubs PRIVATE -Wall -Wextra -g ${SANITIZE_FLAGS})

# Deliberately built WITHOUT sys/include on its path: the project's own
# <stdio.h> is a minimal freestanding subset (no fprintf/stderr/FILE), so
# this is the one place that needs the real host <stdio.h>.
add_library(test_io STATIC test_io.c)
target_compile_options(test_io PRIVATE -Wall -Wextra -g ${SANITIZE_FLAGS})

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
target_compile_options(libk_tests PRIVATE -Wall -Wextra -fno-builtin -g ${SANITIZE_FLAGS})
target_link_options(libk_tests PRIVATE ${SANITIZE_FLAGS})
target_link_libraries(libk_tests PRIVATE libk_under_test kstubs test_io)

enable_testing()
add_test(NAME libk_tests COMMAND libk_tests)

# =========================================================================
# sys/kernel/src/kernel_args.cpp -- kernel-side (not libk) memory-map
# bookkeeping, compiled and linked unmodified, same principle as libk above.
# No symbol renaming needed here: unlike libk, this code doesn't call
# anything on ASan's well-known-libc-name interceptor list.
# =========================================================================

set(KERNEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sys/kernel")

add_library(kernel_args_under_test STATIC
    "${KERNEL_DIR}/src/kernel_args.cpp"
)

target_include_directories(kernel_args_under_test PRIVATE
    "${SYS_INCLUDE_DIR}"
)

target_compile_options(kernel_args_under_test PRIVATE
    -Wall -Wextra -Wpedantic -g ${SANITIZE_FLAGS}
)

# kpanic()/Debug::PrintF(): see kernel_args_stubs.cpp for why this doesn't
# get sys/include on its path.
add_library(kernel_args_stubs STATIC kernel_args_stubs.cpp)
target_compile_options(kernel_args_stubs PRIVATE -Wall -Wextra -g ${SANITIZE_FLAGS})

add_executable(kernel_args_tests
    test_kernel_args.cpp
)

target_include_directories(kernel_args_tests PRIVATE "${SYS_INCLUDE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(kernel_args_tests PRIVATE -Wall -Wextra -g ${SANITIZE_FLAGS})
target_link_options(kernel_args_tests PRIVATE ${SANITIZE_FLAGS})
target_link_libraries(kernel_args_tests PRIVATE kernel_args_under_test kernel_args_stubs test_io)

add_test(NAME kernel_args_tests COMMAND kernel_args_tests)

# =========================================================================
# sys/include/string_view -- header-only, so there's no "under test" source
# to compile separately: the test file includes the real header directly.
# It still needs a real memcmp/memchr/strlen/memcpy behind it (not just
# declarations) and kpanic() for its panic paths, so this links against the
# same libk_under_test/kstubs used above -- which also means it inherits
# ASan/UBSan, catching real out-of-bounds reads rather than only wrong
# answers.
# =========================================================================

add_executable(string_view_tests
    test_string_view.cpp
)

target_include_directories(string_view_tests PRIVATE "${SYS_INCLUDE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(string_view_tests PRIVATE ${LIBK_SYMBOL_RENAMES})
target_compile_options(string_view_tests PRIVATE -Wall -Wextra -g ${SANITIZE_FLAGS})
target_link_options(string_view_tests PRIVATE ${SANITIZE_FLAGS})
target_link_libraries(string_view_tests PRIVATE libk_under_test kstubs test_io)

add_test(NAME string_view_tests COMMAND string_view_tests)
