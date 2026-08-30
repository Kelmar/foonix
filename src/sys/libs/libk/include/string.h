/********************************************************************************************************************/

#ifndef _LIBK_STRING_H
#define _LIBK_STRING_H 1

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

#include <knames.h>

#if !__foonix_libc
# error Did not get the FooNIX sys/cdefs.h!
#endif

/********************************************************************************************************************/

__BEGIN_EXTERN_C

int PREFIX_(memcmp)(const void*, const void*, size_t);
void* PREFIX_(memcpy)(void* __restrict, const void* __restrict, size_t);
void* PREFIX_(memmove)(void*, const void*, size_t);
void* PREFIX_(memset)(void*, int, size_t);
void* PREFIX_(memchr)(const void*, int, size_t);
size_t PREFIX_(strlen)(const char*);
size_t PREFIX_(strnlen)(const char*, size_t);

char* PREFIX_(strncpy)(char*, const char*, size_t);
char* PREFIX_(strncat)(char*, const char*, size_t);

// Semi-nonstandard methods
int PREFIX_(strncmp)(const char *, const char *, size_t);

// Nonstandard methods
char* _s_strncat(char*, size_t, const char*, size_t);
char* _s_strncpy(char*, size_t, const char*, size_t);

__END_EXTERN_C

/********************************************************************************************************************/

#endif /* _LIBK_STRING_H */

/********************************************************************************************************************/
