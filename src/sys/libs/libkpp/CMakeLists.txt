# =========================================================================
# Basic C++ STL implementation library
# =========================================================================

project (libcpp C CXX ASM)

# =========================================================================

list(APPEND SOURCES 
    "rt/runtime.c"
    "rt/cppfill.cpp"
)

# =========================================================================

set(HOST_DIR "${CMAKE_SOURCE_DIR}/sys/libs/libkpp/arch/${PLATFORM}")

if (EXISTS "${HOST_DIR}/build.cmake")
    include("${HOST_DIR}/build.cmake")
endif()

# =========================================================================

add_library(kpp STATIC ${SOURCES})

target_compile_definitions(kpp PUBLIC __is_libk)

target_include_directories(kpp
    PUBLIC
        "${CMAKE_SOURCE_DIR}/sys/libs/libkpp/include"
    PRIVATE
        "${CMAKE_SOURCE_DIR}/sys/include"
)

target_compile_options(kpp PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>: -fno-rtti -fno-exceptions> -ggdb ${COMPILER_EXTRA}
)

target_link_options(kpp PRIVATE -nostdlib ${LINK_EXTRA})
target_link_libraries(kpp PRIVATE k)

# =========================================================================
