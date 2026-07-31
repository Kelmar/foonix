/********************************************************************************************************************/
/*
 * Multiboot 2 definitions.
 *
 * Used by C++ and ASM code.
 */
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_MB2_DEFS_H__
#define __FOONIX_KERNEL_MB2_DEFS_H__

/********************************************************************************************************************/

// Multiboot 2 constants

/** @brief Magic value for Multiboot header detection. */
#define MB2_HDR_MAGIC 0xE85250D6

/** @brief Magic value for Multiboot2 start detection. */
#define MB2_MAGIC 0x36D76289

#define MB2_ARCH_I386 0

/* Tags */
#define MB2_TAG_END         0 /* Terminator tag */
#define MB2_TAG_FRAMEBUFFER 5

/* Tag flags */
#define MB2_TF_NONE     0 /* No flags for this tag */
#define MB2_TF_OPTIONAL 1

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_MB2_DEFS_H__ */

/********************************************************************************************************************/
