# =========================================================================
# Kernel library i386 specific implementations
# =========================================================================

file(GLOB string_asms "${HOST_DIR}/string/*.S")

list(APPEND platform_files ${string_asms})

