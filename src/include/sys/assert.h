/********************************************************************************************************************/

#ifndef __ASSERT_H__
#define __ASSERT_H__ 1

/********************************************************************************************************************/


#if defined(__is_foo_kernel) || defined(__is_libk)

# include <kassert.h>

#else
// Non kernel version, display an error and calls abort()

#include <sys/cdefs.h>


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
