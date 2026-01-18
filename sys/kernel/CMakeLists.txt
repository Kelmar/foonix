cmake_minimum_required(VERSION 3.20)

project(kernel C CXX ASM)

set(HOST_DIR "${CMAKE_SOURCE_DIR}/sys/kernel/arch/${TARGET_HOST}")

include_directories("${CMAKE_SOURCE_DIR}/sys/include")

include("${HOST_DIR}/build.cmake")

file(GLOB_RECURSE KERN_SOURCES *.cpp)

#list(APPEND INC_DIRS "${CAMKE_SOURCE_DIR}/sys/kernel/include")

list(APPEND SOURCES ${KERN_SOURCES})

add_executable(kernel ${SOURCES})
add_dependencies(kernel k)

target_compile_options(kernel PRIVATE -fno-rtti -fno-exceptions -ggdb)

target_link_options(kernel PRIVATE -nostdlib ${LINK_EXTRA})

target_link_libraries(kernel PRIVATE k)

install(
    TARGETS kernel RUNTIME
    DESTINATION ${SYSROOT}/boot
)
