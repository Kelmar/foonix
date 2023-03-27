/*************************************************************************/

#ifndef _STDIO_H
#define _STDIO_H 1

/*************************************************************************/

#include <sys/cdefs.h>
#include <stdarg.h>
#include <stddef.h>

#define EOF (-1)

__BEGIN_EXTERN_C

int vsnprintf(char*, size_t, const char* __restrict, va_list args);
int snprintf(char*, size_t, const char* __restrict, ...);
int vprintf(const char* __restrict, va_list args);
int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

__END_EXTERN_C

/*************************************************************************/

#endif /* _STDIO_H */

/*************************************************************************/
