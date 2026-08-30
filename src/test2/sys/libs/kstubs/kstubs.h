#ifndef TEST_KSTUB_H__
#define TEST_KSTUB_H__

/*
 * These names are redefined with our macros, but we can't include the system libraries in our tests.
 *
 * In some cases we forward them to a stub function to make it easy.  In other cases, they are builtins
 * with the compiler and we just need to provide a good prototype for them.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void fwd_printf(const char *fmt, ...);

int strcmp(const char *, const char *);
size_t strlen(const char *);
int memcmp(const void *, const void *, size_t);
void *memchr(const void *, int, size_t);
void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);
char *strncpy(char *, const char *, size_t);

#ifdef __cplusplus
}
#endif

#include "ktests.h"

#endif /* TEST_KSTUB_H__ */

