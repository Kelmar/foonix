/********************************************************************************************************************/

#ifndef __ASSERT_H__
#define __ASSERT_H__ 1

/********************************************************************************************************************/

/*
 * ASSERT macro, checks for COND_ to be true, if false, then we call abort (kpanic) with an ASSERT failure and provide
 * debugging information.
 */

#ifdef __cplusplus
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif

EXTERN_C __attribute__((noreturn))
void kpanic(const char *message);

EXTERN_C __attribute__((noreturn))
void kassert(const char *test, const char *reason, int line, const char *file, const char *function);

#define ASSERT(TEST_, REASON_) \
    (TEST_) ? (void)(0) : kassert(#TEST_, REASON_, __LINE__, __FILE__, __FUNCTION__)

/********************************************************************************************************************/

#endif /* __ASSERT_H__ */

/********************************************************************************************************************/
