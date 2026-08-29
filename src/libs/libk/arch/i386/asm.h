/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_I386_ASM_H__
#define __FOONIX_ARCH_I386_ASM_H__

/********************************************************************************************************************/

#ifdef __ELF__
# define _C_LABEL(X) X
#else
# ifdef __STDC__
#  define _C_LABEL(X) _ ## X
# else
#  define _C_LABEL(X) _/**/X
# endif
#endif

#ifdef __STDC__
# define __CONCAT(X,Y) X ## Y
# define __STRING(X) #X
#else
# define __CONCAT(X,Y) X/**/Y
# define __STRING(X) "X"
#endif

#ifdef _TEST_
# define __PREFIX(X) __CONCAT(_foo_,X)
#else
# define __PREFIX(X) X
#endif

#if !defined(_ALIGN_TEXT) && !defined(_KERNEL)
# ifdef _STANDALONE
#  define _ALIGN_TEXT .align 4
# elif defined __ELF__
#  define _ALIGN_TEXT .align 16
# else
#  define _ALIGN_TEXT .align 4
# endif
#endif

#define _ENTRY(X)                \
    .text; _ALIGN_TEXT;          \
    .globl __PREFIX(X);          \
    .type __PREFIX(X),@function; \
    __PREFIX(X):

#define _LABEL(X)       \
    .globl __PREFIX(X); \
    __PREFIX(X):

#define NENTRY(Y)	_ENTRY(_C_LABEL(Y))

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_I386_ASM_H__ */

/********************************************************************************************************************/
