/*************************************************************************/

#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

/*************************************************************************/

__BEGIN_EXTERN_C

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
void* memchr(const void*, int, size_t);
size_t strlen(const char*);
size_t strnlen(const char*, size_t);

char* strncpy(char*, const char*, size_t);
char* strncat(char*, const char*, size_t);

char* _s_strncat(char*, size_t, const char*, size_t);

__END_EXTERN_C

/*************************************************************************/

#endif /* _STRING_H */

/*************************************************************************/
