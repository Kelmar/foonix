/********************************************************************************************************************/

#ifndef __ASSERT_H__
#define __ASSERT_H__ 1

/********************************************************************************************************************/

/*
 * ASSERT macro, checks for COND_ to be true, if false, then we call abort (panic) with an ASSERT failure and provide
 * debugging information.
 */

#include <stdio.h>

#ifdef __cplusplus
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif

EXTERN_C __attribute__((noreturn))
void kpanic(const char *msg);

#define ASSERT(COND_, REASON_)              \
  do { if (!(COND_)) {			                \
    char buf[1024];			                    \
    snprintf(buf, sizeof(buf),		          \
      "\n**** ASSERTION FAILURE ****\n"	    \
      "    CONDITION: %s\n"		              \
      "    FILE     : %s\n"		              \
      "    LINE     : %d\n"		              \
      "    REASON   : %s\n"		              \
      "**** ASSERTION FAILURE ****\n",	    \
      #COND_, __FILE__, __LINE__, REASON_); \
      kpanic(buf); } } while (false)

/********************************************************************************************************************/

#endif /* __ASSERT_H__ */

/********************************************************************************************************************/
