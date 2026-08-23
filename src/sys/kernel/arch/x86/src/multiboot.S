/********************************************************************************************************************/
/*
 * Information for populating the Multiboot 1 & 2 headers.
 */
/********************************************************************************************************************/

#include "mb_defs.h"
#include "mb2_defs.h"

/********************************************************************************************************************/

#define MB1_FLAGS (MB_REQ_ALIGN | MB_REQ_MEMINFO)

/********************************************************************************************************************/

# Multiboot header
.section .multiboot, "aw"
mb1_start:
.align 4
.long MB_MAGIC
.long MB1_FLAGS
.long -(MB_MAGIC + MB1_FLAGS)

.align 8
mb2_start:
.long MB2_HDR_MAGIC
.long MB2_ARCH_I386
.long mb2_end - mb2_start
.long 0x100000000 - (MB2_HDR_MAGIC + MB2_ARCH_I386 + (mb2_end - mb2_start))

/*
.align 8
.word MB2_TAG_FRAMEBUFFER
.word MB2_TF_OPTIONAL
.long 20   // Tag size
.long 1024 // Width
.long 768  // Height
.long 32   // Depth (bits per pixel)
*/

.align 8
.word MB2_TAG_END
.word MB2_TF_NONE
.long 8  // Tag size
mb2_end:

/********************************************************************************************************************/
