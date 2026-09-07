# =========================================================================
# Kernel library++ i386 specific implementations
# =========================================================================

list(APPEND COMPILER_EXTRA -fno-pie)

list(APPEND LINK_EXTRA "-T${HOST_DIR}/linker.ld" -no-pie)
