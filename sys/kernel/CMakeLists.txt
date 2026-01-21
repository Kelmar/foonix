# =========================================================================
# Kernel global CMakeLists
# =========================================================================

cmake_minimum_required(VERSION 3.25)

project(kernel C CXX ASM)

set(KERNEL_SRC_DIR "${CMAKE_SOURCE_DIR}/sys/kernel")
set(HOST_DIR "${KERNEL_SRC_DIR}/arch/${PLATFORM}")

include_directories("${CMAKE_SOURCE_DIR}/sys/include")

include("${HOST_DIR}/build.cmake")

include("${CMAKE_SOURCE_DIR}/cmake/klink.cmake")

# =========================================================================

list(APPEND KERN_SOURCES
    src/debug.cpp src/kernel_args.cpp src/main.cpp
    src/allocator.cpp src/vm/vm_page.cpp src/vm/vm.cpp
)

list(TRANSFORM KERN_SOURCES PREPEND "${KERNEL_SOURCE_DIR}")

#list(APPEND INC_DIRS "${CAMKE_SOURCE_DIR}/sys/kernel/include")

list(APPEND SOURCES ${KERN_SOURCES})

# =========================================================================

add_executable(kernel ${SOURCES})
add_dependencies(kernel k)

include("${CMAKE_SOURCE_DIR}/cmake/klink.cmake")

target_compile_options(kernel PRIVATE -fno-rtti -fno-exceptions -ggdb)

target_link_options(kernel PRIVATE -nostdlib ${LINK_EXTRA})

target_link_libraries(kernel PRIVATE k)

install(
    TARGETS kernel RUNTIME
    DESTINATION ${SYSROOT}/boot
)

# =========================================================================
