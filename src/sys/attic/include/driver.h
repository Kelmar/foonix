/*************************************************************************/
/*
 * $Id$
 */
/*************************************************************************/

#ifndef _FOO_DRIVER_H_
#define _FOO_DRIVER_H_

/*************************************************************************/

#include "stdint.h"
#include "cdefs.h"

struct _DRIVER_INTERFACE_TYPE
{
    const char *name;
    bool_t (*initialize)(void);
    void   (*shutdown)(void);
};

typedef struct _DRIVER_INTERFACE_TYPE driver_interface_t;

/*************************************************************************/

#endif /* _FOO_DRIVER_H_ */

/*************************************************************************/
