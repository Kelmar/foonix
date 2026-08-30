# =========================================================================
# Unit tests for libk
# =========================================================================

project (foonix_libk_tests C CXX ASM)

# =========================================================================

set(LIBK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../sys/libs/libk")

# =========================================================================

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

target_include_directories(libk_under_test
    PUBLIC
        "${GLOBAL_INCLUDE_DIR}"
        "${LIBK_DIR}/include"
        "${LIBKPP_DIR}/include"
    PRIVATE
        "${SYS_INCLUDE_DIR}"
)

target_compile_options(libk_under_test PRIVATE
    -Wall -Wextra -Wpedantic -fno-builtin -g ${SANITIZE_FLAGS}
)

#
# Give our functions a different name so that they don't conflict with the 
# host version of the same name.  Also will keep various memory checkers
# (like ASan) from trying to intercept calls to our functions.
#
target_compile_definitions(libk PRIVATE PREFIX__=foonix_)

# =========================================================================

add_executable(libk_tests
    test_bounded_string.c
    test_memory.c
    test_strlen.c
    test_vsnprintf.c
    kstubs.c
    main.c
)

enable_testing()

add_test(NAME libk_tests COMMAND libk_tests)

# =========================================================================
