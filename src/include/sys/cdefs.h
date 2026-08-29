/********************************************************************************************************************/

#ifndef _SYS_CDEFS_H
#define _SYS_CDEFS_H 1

/********************************************************************************************************************/

#define __foonix_libc 1

#if !defined(__GNUC_PREREQ__)

#ifdef __GNUC__
# define __GNUC_PREREQ__(x, y)			    \
    ((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) ||  \
     (__GNUC__ > (x)))
#else
# define __GNUC_PREREQ__(x, y) 0
#endif

#endif

#ifdef __cplusplus
# define __BEGIN_EXTERN_C extern "C" {
# define __END_EXTERN_C }
#else
# define __BEGIN_EXTERN_C
# define __END_EXTERN_C
#endif

#define FORCE_INLINE __attribute__((always_inline))

#define UNUSED(X) (void)(X)

/********************************************************************************************************************/

#endif /* _SYS_CDEFS_H */

/********************************************************************************************************************/
