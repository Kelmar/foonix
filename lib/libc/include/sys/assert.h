/********************************************************************************************************************/

#ifndef __ASSERT_H__
#define __ASSERT_H__ 1

/********************************************************************************************************************/

/*
 * ASSERT macro, checks for COND_ to be true, if false, then we call abort (panic) with an ASSERT failure and provide
 * debugging information.
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef __is_libk
//# include <kernel/flow.h>

// TODO: Put back into a kernel header file.
__attribute__((__noreturn__))
void panic(const char* message);

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
      abort(); } } while (false)
#endif

/********************************************************************************************************************/

#endif /* __ASSERT_H__ */

/********************************************************************************************************************/
