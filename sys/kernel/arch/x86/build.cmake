# =========================================================================
# Kernel x86 specific files
# =========================================================================

# The files in this directory are for common functionality between 32-bit
# and 64-bit platforms.

# You should be buliding for either i386 or for x86_64

if (NOT X86_BUILD)
    message(FATAL_ERROR "x86 is a dummy platform!\nplease build either i386 or x86_64")
endif()

# =========================================================================

list(APPEND x86_sources
    dconsole.cpp
)

list(TRANSFORM x86_sources PREPEND "${KERNEL_SRC_DIR}/arch/x86/src/")
list(APPEND SOURCES ${x86_sources})

# =========================================================================

include_directories("${KERNEL_SRC_DIR}/arch/x86/include")

# =========================================================================
