# =========================================================================
# Kernel x86-64 specific build options
# =========================================================================

set(X86_BUILD true)
include ("${KERNEL_SRC_DIR}/arch/x86/build.cmake")

# =========================================================================

include_directories("${HOST_DIR}/include")

# compiler-rt Not needed on x86-64?

#execute_process(
#    COMMAND clang -target x86_64-unknown-none-elf -print-libgcc-file-name --rtlib=compiler-rt
#    OUTPUT_VARIABLE RT_PATH
#    OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#list(APPEND LINK_EXTRA "${RT_PATH}")

list(APPEND x64_sources
    start.S boot.S cpu.S paging.cpp page_table.cpp
)

list(TRANSFORM x64_sources PREPEND "${HOST_DIR}/src/")
list(APPEND SOURCES ${x64_sources})

list(APPEND LINK_EXTRA "-T${HOST_DIR}/linker.ld" -no-pie)
list(APPEND COMPILER_EXTRA "-fno-pie")

# =========================================================================
