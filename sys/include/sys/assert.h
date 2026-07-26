/********************************************************************************************************************/

#ifndef __ASSERT_H__
#define __ASSERT_H__ 1

/********************************************************************************************************************/

#include <sys/cdefs.h>

#if defined(__is_foo_kernel) || defined(__is_libk)
// Pull in functions for kpanic and kassert
# include "kernel/flow.h"

/*
 * ASSERT macro, checks for COND_ to be true, if false, then we call abort (kpanic) with an ASSERT failure and provide
 * debugging information.
 */

# define ASSERT(TEST_, REASON_) \
    (TEST_) ? (void)(0) : kassert(#TEST_, REASON_, __LINE__, __FILE__, __func__)
#else
// Non kernel version, display an error and calls abort()

# define ASSERT(TEST_, REASON_) \
    do { if (TEST_) { } else { fprintf(stderr, \
"\r\n********** ASSERT FAIL ***********\r\n\
    REASON: %s\r\n\
    TEST: %s\r\n\
    LOCATION: %s (%s:%d)\r\n\
********** ASSERT FAIL ***********\r\n\r\n", \
            REASON_, #TEST_, __func__, __LINE__, __FILE__); \
        abort(); \
    } } while (false)
#endif

/********************************************************************************************************************/

#endif /* __ASSERT_H__ */

/********************************************************************************************************************/
