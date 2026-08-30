# =========================================================================
# Basic C++ STL implementation library
# =========================================================================

project (libcpp C CXX ASM)

# =========================================================================

list(APPEND SOURCES 
    "rt/runtime.c"
    "rt/cppfill.cpp"
)

add_library(kpp STATIC ${SOURCES})

target_compile_definitions(kpp PUBLIC __is_libk)

target_include_directories(kpp
    PUBLIC
        "${CMAKE_SOURCE_DIR}/sys/libs/libkpp/include"
    PRIVATE
        "${CMAKE_SOURCE_DIR}/sys/include"
)

target_compile_options(kpp PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>: -fno-rtti -fno-exceptions> -ggdb
)

target_link_options(k PRIVATE -nostdlib)
#target_link_libraries(k PRIVATE gcc)

# =========================================================================
