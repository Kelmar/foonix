/*************************************************************************/
/*
 * $Id: string.h 44 2010-02-19 23:53:25Z kfiresun $ 
 */
/*************************************************************************/

#ifndef _FOO_STRING_H_
#define _FOO_STRING_H_

/*************************************************************************/

#include "cdefs.h"

#if !defined(_TEST_)
# include "stdint.h"
# define _DCL(X) X
#else
# define _DCL(X) _foo_ ## X
#endif

/*************************************************************************/

__BEGIN_DECLS

/* Non-standard functions */
void *_DCL(_memsetw)(void *dst, uint16_t word, size_t sz);

char *_DCL(strkcat)(char *, size_t, const char *, size_t);
char *_DCL(strkcpy)(char *, size_t, const char *, size_t);

char *_DCL(strbswp)(char *, size_t);
char *_DCL(strtrim)(char *, size_t);

/* Quazi-standard functions */
/* These aren't strictly defined, but common across many systems. */
size_t _DCL(strnlen)(const char *, size_t);

/* Standard functions */
void *_DCL(memset)(void *, int, size_t);
void *_DCL(memcpy)(void *, const void *, size_t);
void *_DCL(memchr)(const void *, int, size_t);

/*
 * Ugh, very unsafe, checks 1k at most....
 */
#define strlen(X) strnlen(X, (size_t)1024)

__END_DECLS

/*************************************************************************/

#undef _DCL

/*************************************************************************/

#endif /* _FOO_STRING_H_ */

/*************************************************************************/
