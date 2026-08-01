/*************************************************************************/
/*************************************************************************/

#ifndef _KTIME_H_
#define _KTIME_H_

/*************************************************************************/

#include "stdarg.h"

/*************************************************************************/

/*
 * It would be nice to make this opaque, but we can't do that until we
 * get a working memory manager.....
 */
struct ktimer_TYPE
{
    uint32_t	counter;
    uint32_t	last_read;
    uint32_t	target;
    uint32_t	msecs;
};

typedef struct ktimer_TYPE ktimer_t;

/*************************************************************************/

__BEGIN_DECLS

void init_timer(void);

/*
 * Initializes a kernel asyncronous oneshot timer.
 *
 * Parameters:
 *   timer  - Pointer to a ktimer_t structure.
 *   msecs  - The time in milisecons this timer is for.
 */
void init_ktimer(ktimer_t *timer, uint32_t msecs);

/*
 * Updates and checks the status of a kernel asyncronous timer.
 *
 * Parameters:
 *    timer  - Pointer to a ktimer_t structure to check.
 *
 * Returns:
 *    true if there is time left in this timer, false if not.
 */
bool_t check_ktimer(ktimer_t *timer);

/*
 * Resets an already initilized ktimer so it can be used again.
 */
void reset_ktimer(ktimer_t *timer);

__END_DECLS


/*************************************************************************/

#endif /* _KTIME_H_ */

/*************************************************************************/
