/********************************************************************************************************************/
/*
 * Information for populating the Multiboot 1 & 2 headers.
 */
/********************************************************************************************************************/

#include "mb_defs.h"
#include "mb2_defs.h"

/********************************************************************************************************************/

// #define MB1_FLAGS (MB_REQ_ALIGN | MB_REQ_MEMINFO | MB_REQ_VIDEO)
#define MB1_FLAGS (MB_REQ_ALIGN | MB_REQ_MEMINFO)

/********************************************************************************************************************/

# Multiboot header
.section .multiboot, "aw"
mb1_start:
.align 4
.long MB_MAGIC
.long MB1_FLAGS
.long -(MB_MAGIC + MB1_FLAGS)

# Flags 16 (ignore exe header values)
.long 0 # Header address
.long 0 # Load address
.long 0 # Load end address
.long 0 # BSS end address
.long 0 # Entry address

# Video mode requested
.long 0    # Video mode type (0 = linear, 1 = EGA text)
.long 1280 # Width
.long 800  # Height
.long 24   # Depth (bits per pixel)

# Multiboot 2 Header
.align 8
mb2_start:
.long MB2_HDR_MAGIC
.long MB2_ARCH_I386
.long mb2_end - mb2_start
.long 0x100000000 - (MB2_HDR_MAGIC + MB2_ARCH_I386 + (mb2_end - mb2_start))

/*
.align 8
.word MB2_HDR_TAG_FRAMEBUFFER
.word MB2_TF_OPTIONAL
.long 20   // Tag size
.long 1280 // Width
.long 800  // Height
.long 24   // Depth (bits per pixel)
*/

.align 8
.word MB2_TAG_END
.word MB2_TF_NONE
.long 8  // Tag size
mb2_end:

/********************************************************************************************************************/
