# =========================================================================
# Kernel global CMakeLists
# =========================================================================

cmake_minimum_required(VERSION 3.25)

project(kernel C CXX ASM)

set(KERNEL_SRC_DIR "${CMAKE_SOURCE_DIR}/sys/kernel")
set(HOST_DIR "${KERNEL_SRC_DIR}/arch/${PLATFORM}")

include_directories("${CMAKE_SOURCE_DIR}/sys/include")

include("${HOST_DIR}/build.cmake")

# =========================================================================

list(APPEND KERN_SOURCES
    src/debug.cpp src/kernel_args.cpp src/main.cpp src/console.cpp
    src/allocator.cpp src/vm/vm_page.cpp src/vm/vm.cpp
)

list(TRANSFORM KERN_SOURCES PREPEND "${KERNEL_SOURCE_DIR}")

#list(APPEND INC_DIRS "${CAMKE_SOURCE_DIR}/sys/kernel/include")

list(APPEND SOURCES ${KERN_SOURCES})

# =========================================================================

add_executable(kernel ${SOURCES})
add_dependencies(kernel k)

set_target_properties(kernel PROPERTIES OUTPUT_NAME "kernel.elf")

#target_assembler_options(kernel PRIVATE ${ASSEMBLE_EXTRA} ${ALL_EXTRA})

target_compile_options(kernel PRIVATE 
    -fno-rtti
    -fno-exceptions
    -ggdb
    ${COMPILE_EXTRA}
    ${ALL_EXTRA}
    -D__is_foo_kernel)

target_link_options(kernel PRIVATE -nostdlib ${ALL_EXTRA} ${LINK_EXTRA})

#target_link_libraries(kernel PRIVATE k gcc)
target_link_libraries(kernel PRIVATE k ${LINK_LIBS})

install(
    TARGETS kernel RUNTIME
    DESTINATION ${SYSROOT}/boot
)

# =========================================================================
