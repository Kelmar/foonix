/*************************************************************************/

#ifndef _STDLIB_H
#define _STDLIB_H 1

/*************************************************************************/

#include <sys/cdefs.h>
#include <stddef.h>

__BEGIN_EXTERN_C

__attribute__((__noreturn__))
void abort(void);

void* malloc(size_t size);
void free(void* ptr);

__END_EXTERN_C

/*************************************************************************/

#endif /* _STDLIB_H */

/*************************************************************************/
