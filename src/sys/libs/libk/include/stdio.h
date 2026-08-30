/********************************************************************************************************************/

#ifndef _LIBK_STDIO_H
#define _LIBK_STDIO_H 1

/********************************************************************************************************************/

#include <sys/cdefs.h>
#include <stdarg.h>
#include <stddef.h>

#include <knames.h>

#if !__foonix_libc
# error Did not get the FooNIX sys/cdefs.h!
#endif

/********************************************************************************************************************/

#define EOF (-1)

__BEGIN_EXTERN_C

int PREFIX_(vsnprintf)(char* __restrict, size_t, const char* __restrict, va_list args);
int PREFIX_(snprintf)(char* __restrict, size_t, const char* __restrict, ...);
int PREFIX_(vprintf)(const char* __restrict, va_list args);
int PREFIX_(printf)(const char* __restrict, ...);
int PREFIX_(putchar)(int);
int PREFIX_(puts)(const char*);

__END_EXTERN_C

/********************************************************************************************************************/

#endif /* _LIBK_STDIO_H */

/********************************************************************************************************************/
