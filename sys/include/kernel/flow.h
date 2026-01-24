/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_FLOW_H__
#define __FOONIX_KERNEL_FLOW_H__

/********************************************************************************************************************/

#include <sys/cdefs.h>

/********************************************************************************************************************/

__BEGIN_EXTERN_C

/// @brief Halt the system.
__attribute__((__noreturn__))
void khalt(void); // Defined by architecture

/// @brief Display a "STOP ERROR" message and halt the system.
__attribute__((__noreturn__))
void kpanic(const char* message);

/// @brief Display a "STOP ERROR" message and halt the system.
__attribute__((__noreturn__))
void kassert(const char *test, const char *reason, int line, const char *file, const char *function);

#define ASSERT(TEST_, REASON_) \
    (TEST_) ? (void)(0) : kassert(#TEST_, REASON_, __LINE__, __FILE__, __FUNCTION__)

__END_EXTERN_C

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_FLOW_H__ */

/********************************************************************************************************************/
