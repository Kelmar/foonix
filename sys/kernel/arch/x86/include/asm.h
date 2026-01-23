/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_I386_ASM_H__
#define __FOONIX_ARCH_I386_ASM_H__

/********************************************************************************************************************/

#include "defs.h"

#ifdef __STDC__
# define __CONCAT(X,Y) X ## Y
# define __STRING(X) #X
#else
# define __CONCAT(X,Y) X/**/Y
# define __STRING(X) "X"
#endif

#ifdef __ELF__
# define _C_LABEL(X) X
#else
# ifdef __STDC__
#  define _C_LABEL(X) _ ## X
# else
#  define _C_LABEL(X) _/**/X
# endif
#endif

#ifdef _TEST_
# define __PREFIX(X) __CONCAT(_foo_,X)
#else
# define __PREFIX(X) X
#endif

#define _ENTRY(X)                \
    .text; _ALIGN_TEXT;          \
    .globl __PREFIX(X);          \
    .type __PREFIX(X),@function; \
    __PREFIX(X):

#define _LABEL(X)       \
    .globl __PREFIX(X); \
    __PREFIX(X):

#define NENTRY(X) _ENTRY(_C_LABEL(X))
#define LABEL(X)  _LABEL(_C_LABEL(X))

#define VIRT_2_PHYS(X_) (X_ - KERNEL_OFFSET)
#define PHYS_2_VIRT(X_) (X_ + KERNEL_OFFSET)

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_I386_ASM_H__ */

/********************************************************************************************************************/
