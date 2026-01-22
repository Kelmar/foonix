# =========================================================================
# Kernel i386 specific build options
# =========================================================================

set(X86_BUILD true)
include ("${KERNEL_SRC_DIR}/arch/x86/build.cmake")

# =========================================================================

# Add platform specific files
list(APPEND i386_sources
    start.S boot.S cpu.S isr.S memsetw.S vectors.s
    bus.cpp gdt.cpp idt.cpp paging.cpp preinit.cpp
    arch_vm.cpp
)

list(TRANSFORM i386_sources PREPEND "${HOST_DIR}/i386/")
list(APPEND SOURCES ${i386_sources})

# Add platform specific device drivers.
list(APPEND SOURCES
    "${HOST_DIR}/dev/timer.cpp"
)

# CRT platform specific files.
list(APPEND SOURCES
    "${HOST_DIR}/crti.S"
    "${HOST_DIR}/crtn.S"
)

# =========================================================================
# Setup include and linker options

include_directories("${HOST_DIR}/include")

list(APPEND LINK_EXTRA "-T${HOST_DIR}/linker.ld")

# =========================================================================
