# =========================================================================
# Kernel i386 specific build options
# =========================================================================

set(X86_BUILD true)
include ("${KERNEL_SRC_DIR}/arch/x86/build.cmake")

# =========================================================================

# Add platform specific files
list(APPEND i386_sources
    boot.S cpu.S isr.S memsetw.S vectors.s
    bus.cpp gdt.cpp idt.cpp paging.cpp preinit.cpp
    realmem.cpp
)

list(TRANSFORM i386_sources PREPEND "${HOST_DIR}/src/")
list(APPEND SOURCES ${i386_sources})

# Add platform specific device drivers.
list(APPEND SOURCES
    "${HOST_DIR}/dev/timer.cpp"
)

# =========================================================================
# Setup include and linker options

# Find compiler-rt libraries to add back in for i386
execute_process(
    COMMAND clang++ -m32 --rtlib=compiler-rt -print-libgcc-file-name
    OUTPUT_VARIABLE RT_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

include_directories("${HOST_DIR}/include")

list(APPEND COMPILER_EXTRA "-fno-pie")

list(APPEND LINK_EXTRA "-T${HOST_DIR}/linker.ld" -no-pie)
list(APPEND LINK_LIBS "${RT_PATH}")

# =========================================================================
