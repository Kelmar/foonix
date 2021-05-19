/*************************************************************************/
/*
 * $Id: cdefs.h 62 2010-07-08 01:30:35Z kfiresun $
 */
/*************************************************************************/

#ifndef _CDEFS_H_
#define _CDEFS_H_

/*************************************************************************/

#if !defined(__GNUC_PREREQ__)

#ifdef __GNUC__
# define __GNUC_PREREQ__(x, y)			    \
    ((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) ||  \
     (__GNUC__ > (x)))
#else
# define __GNUC_PREREQ__(x, y) 0
#endif

#endif

#define UNUSED(X) (void)(X)

/*************************************************************************/

#ifdef __cplusplus

# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }

typedef bool bool_t;

#else

# define __BEGIN_DECLS
# define __END_DECLS

typedef enum bool_TYPE
{
    false = 0,
    true
} bool_t;

#endif

/*************************************************************************/

#endif /* _CDEFS_H_ */

/*************************************************************************/
