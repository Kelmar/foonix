cmake_minimum_required(VERSION 3.20)

# =========================================================================

project (foonix_tests)

include(FetchContent)

# =========================================================================

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.5.3
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(doctest)

# =========================================================================

add_subdirectory("sys/libs/libk/tests")

# =========================================================================
