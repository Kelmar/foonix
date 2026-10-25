# =========================================================================
# Kernel library++ x86 64-bit specific implementations
# =========================================================================

list(APPEND LINK_EXTRA LINKER:-z,max-page-size=4096 -no-pie -mgeneral-regs-only -mcmodel=kernel)

