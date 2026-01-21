# =========================================================================
# Kernel x86-64 specific build options
# =========================================================================

set(X86_BUILD true)
include ("${KERNEL_SRC_DIR}/arch/x86/build.cmake")

# =========================================================================

include_directories("${HOST_DIR}/include")

list(APPEND x64_sources
    boot.S
    arch_vm.cpp
)

list(TRANSFORM x64_sources PREPEND "${HOST_DIR}/src/")
list(APPEND SOURCES ${x64_sources})

list(APPEND LINK_EXTRA "-T${HOST_DIR}/linker.ld")

# =========================================================================
