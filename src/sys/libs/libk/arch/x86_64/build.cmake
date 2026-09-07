# =========================================================================
# Kernel library x86 64-bit specific implementations
# =========================================================================

list(APPEND LINK_EXTRA LINKER:-T${HOST_DIR}/linker.ld LINKER:-z,max-page-size=4096 -no-pie)
list(APPEND COMPILER_EXTRA -mcmodel=kernel -mno-red-zone -fno-pie)