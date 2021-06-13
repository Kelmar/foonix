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

/*
 * ASSERT macro, checks for COND_ to be true, if false, then we call abort (panic) with an ASSERT failure and provide
 * debugging information.
 */

#ifdef __is_libk
# include <kernel/flow.h>
# define ASSERT(COND_, REASON_)             \
  do { if (!(COND_)) {			    \
    char buf[1024];			    \
    snprintf(buf, sizeof(buf),		    \
      "\n**** ASSERTION FAILURE ****\n"	    \
      "    CONDITION: %s\n"		    \
      "    FILE     : %s\n"		    \
      "    LINE     : %d\n"		    \
      "    REASON   : %s\n"		    \
      "**** ASSERTION FAILURE ****\n",	    \
      #COND_, __FILE__, __LINE__, REASON_); \
      panic(buf); } } while (false)
#else
# define ASSERT(COND_, REASON_)             \
  do { if (!(COND_)) {			    \
    printf(				    \
      "\n**** ASSERTION FAILURE ****\n"	    \
      "    CONDITION: %s\n"		    \
      "    FILE     : %s\n"		    \
      "    LINE     : %d\n"		    \
      "    REASON   : %s\n"		    \
      "**** ASSERTION FAILURE ****\n",	    \
      #COND_, __FILE__, __LINE__, REASON_); \
      assert(); } } while (false)
#endif

#define UNUSED(X) (void)(X)

/********************************************************************************************************************/

#endif /* _SYS_CDEFS_H */

/********************************************************************************************************************/
