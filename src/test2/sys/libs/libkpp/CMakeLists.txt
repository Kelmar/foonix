# =========================================================================
# Unit tests for libkpp
# =========================================================================

project (foonix_libkpp_tests C CXX ASM)

# =========================================================================

set(LIBK_DIR "${FOO_ROOT_DIR}/sys/libs/libk")
set(LIBKPP_DIR "${FOO_ROOT_DIR}/sys/libs/libkpp")

# =========================================================================

add_library(libkpp_under_test STATIC stub.cpp)

target_include_directories(libkpp_under_test
    PUBLIC
        "${GLOBAL_INCLUDE_DIR}"
        "${LIBK_DIR}/include"
        "${LIBKPP_DIR}/include"
    PRIVATE
        "${SYS_INCLUDE_DIR}"
)

target_link_libraries(libkpp_under_test PRIVATE kstubs)

target_compile_options(libkpp_under_test PRIVATE
    -Wall -Wextra -Wpedantic -g ${SANITIZE_FLAGS}
)

#
# Give our functions a different name so that they don't conflict with the 
# host version of the same name.  Also will keep various memory checkers
# (like ASan) from trying to intercept calls to our functions.
#
target_compile_definitions(libkpp_under_test PRIVATE
    __is_libk
    PREFIX__=foonix_
)

# =========================================================================

add_executable(libkpp_tests
    string_view_tests.cpp
)

target_include_directories(libkpp_tests
    PUBLIC
        "${GLOBAL_INCLUDE_DIR}"
        "${LIBK_DIR}/include"
        "${LIBKPP_DIR}/include"
    PRIVATE
        "${SYS_INCLUDE_DIR}"
)

target_link_libraries(libkpp_tests PRIVATE kstubs libkpp_under_test)

target_compile_options(libkpp_tests PRIVATE
    -Wall -Wextra -Wpedantic -g ${SANITIZE_FLAGS}
)

target_compile_definitions(libkpp_tests PRIVATE
    __is_libk
    PREFIX__=foonix_
)

add_test(NAME libkpp_tests COMMAND libkpp_tests)

# =========================================================================
