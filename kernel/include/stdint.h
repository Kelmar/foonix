/*************************************************************************/
/*
 * $Id: stdint.h 44 2010-02-19 23:53:25Z kfiresun $
 */
/*************************************************************************/

#ifndef _STDINT_H_
#define _STDINT_H_

/*************************************************************************/

#define VLONG long long

typedef char	int8_t;
typedef short	int16_t;
typedef int	int32_t;
typedef VLONG	int64_t;

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned VLONG uint64_t;

#undef VLONG

#define INT8_MIN	(-0x7F-1)
#define INT16_MIN	(-0x7FFF-1)
#define INT32_MIN	(-0x7FFFFFFF-1)
#define INT64_MIN	(-0x7FFFFFFFFFFFFFFFLL-1)

#define INT8_MAX	0x7F
#define INT16_MAX	0x7FFF
#define INT32_MAX	0x7FFFFFFF
#define INT64_MAX	0x7FFFFFFFFFFFFFFFLL

#define UINT8_MAX	0xFFU
#define UINT16_MAX	0xFFFFU
#define UINT32_MAX	0xFFFFFFFFU
#define UINT64_MAX	0xFFFFFFFFFFFFFFFFULL

/*************************************************************************/

typedef uint32_t size_t;
typedef int32_t ssize_t;

#define SSIZE_MIN	INT32_MIN
#define SSIZE_MAX	INT32_MAX
#define SIZE_MAX	UINT32_MAX

/*************************************************************************/

#if defined(_FOO64)

/* 64 bit platform */
typedef  int64_t  intptr_t;
typedef uint64_t uintptr_t;

# define INTPTR_MIN	INT64_MIN
# define INTPTR_MAX	INT64_MAX
# define UINTPTR_MAX	UINT64_MAX

#else

/* 32 bit platform */
typedef  int32_t  intptr_t;
typedef uint32_t uintptr_t;

# define INTPTR_MIN	INT32_MIN
# define INTPTR_MAX	INT32_MAX
# define UINTPTR_MAX	UINT32_MAX

#endif /* _FOO64 */

/*************************************************************************/

#endif /* _STDINT_H_ */

/*************************************************************************/

