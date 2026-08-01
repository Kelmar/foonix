/********************************************************************************************************************/
/*
 * Information for populating the Multiboot 2 header.
 */
/********************************************************************************************************************/

#include "mb2_defs.h"

/********************************************************************************************************************/

# Multiboot header
.section .multiboot.data, "aw"
.align 8
multiboot_header:
.long MB2_HDR_MAGIC
.long MB2_ARCH_I386
.long multiboot_header_end - multiboot_header
.long 0x100000000 - (MB2_HDR_MAGIC + MB2_ARCH_I386 + (multiboot_header_end - multiboot_header))

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
multiboot_header_end:

/********************************************************************************************************************/
