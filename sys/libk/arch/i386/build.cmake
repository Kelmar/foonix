# =========================================================================
# Kernel library i386 specific implementations
# =========================================================================

set(string_asms
    memchr.S memcpy.S memset.S
)

list(TRANSFORM string_asms PREPEND "${HOST_DIR}/string/")

list(APPEND platform_files ${string_asms})
