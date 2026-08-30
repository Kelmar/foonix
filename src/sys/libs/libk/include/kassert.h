/********************************************************************************************************************/

#ifndef __KASSERT_H__
#define __KASSERT_H__ 1

/********************************************************************************************************************/

#include <sys/cdefs.h>

#define NO_RETURN __attribute__((__noreturn__))

__BEGIN_EXTERN_C

NO_RETURN
void khalt();

NO_RETURN
void kpanic(const char *message);

NO_RETURN
void kassert(const char *test, const char *reason, int line, const char *file, const char *function);

__END_EXTERN_C

#undef NO_RETURN

/*
 * ASSERT macro, checks for COND_ to be true, if false, then we call abort (kpanic) with an ASSERT failure and provide
 * debugging information.
 */

#define ASSERT(TEST_, REASON_) \
    (TEST_) ? (void)(0) : kassert(#TEST_, REASON_, __LINE__, __FILE__, __func__)

/********************************************************************************************************************/

#endif /* __KASSERT_H__ */

/********************************************************************************************************************/
