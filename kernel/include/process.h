/*************************************************************************/
/*
 * $Id: process.h 44 2010-02-19 23:53:25Z kfiresun $
 */
/*************************************************************************/

#ifndef __FOONIX_PROCESS_H__
#define __FOONIX_PROCESS_H__

/*************************************************************************/

struct process_TYPE;
typedef struct process_TYPE process_t;

typedef void (*process_entry_t)(void);

/*************************************************************************/

/** Start a new process */
void create_process(process_entry_t);

void init_scheduler(void);
void run_scheduler(struct regs *r);

/*************************************************************************/

#endif /* __FOONIX_PROCESS_H__ */

/*************************************************************************/
