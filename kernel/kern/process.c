/*************************************************************************/
/*
 * $Id: process.c 45 2010-02-21 00:20:52Z kfiresun $ 
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "stdlib.h"
#include "string.h"
#include "paging.h"
#include "kheap.h"
#include "process.h"
#include "kernio.h"

/*************************************************************************/

enum task_state_ENUM
{
    TASK_CREATED = 0,
    TASK_RUNNING = 1,
    TASK_WAIT = 2,
    TASK_FINISHED
};

typedef enum task_state_ENUM task_state_e;

/*************************************************************************/

struct process_TYPE
{
    process_t *m_prev; /* Previous process in the list. */
    process_t *m_next; /* Next process in the list. */

    task_state_e m_state;

    int m_slicesLeft;

    struct regs	    m_regs;	 /* Saved registers */

    /* Entry data */
    process_entry_t m_entry;
};

/* Process list */
static process_t *s_firstProc = NULL;
//static process_t *s_waitProc = NULL;
static process_t *s_lastProc = NULL;

static process_t *s_current = NULL;

/* List of finished tasks */
static process_t *s_finished = NULL;

/*************************************************************************/

/* Defined in cpu.S */
void sti(void);
void cli(void);

void process_prestub(void);

/*************************************************************************/
/*
 * The stub function is responsible for starting/stoping the actual process.
 */
void process_stub(void)
{
    process_t *proc = (process_t *)read_eax();

    kprintf("Process %p starting.\n", proc);

    /* Call our actual process function */
    proc->m_state = TASK_RUNNING;
    proc->m_entry();
    proc->m_state = TASK_FINISHED;

    kprintf("Process %p ending.\n", proc);

    /*
     * Remove ourselves from the process lists, and place into the finished
     * list.
     */
    if (proc->m_prev != 0)
	proc->m_prev->m_next = proc->m_next;
    else
	s_firstProc = proc->m_next;

    if (proc->m_next != 0)
	proc->m_next->m_prev = proc->m_prev;
    else
	s_lastProc = proc->m_prev;

    /* 
     * Use the previous process so we don't accidentally steal cylces
     * from the next process.
     */
    s_current = proc->m_prev;

    proc->m_next = s_finished;
    s_finished = proc;

    /* 
     * DON'T RETURN!  The next task switch will clean us up!
     */
    sti(); /* Re-enable interrupts so the next task switch will occur */
    for (;;) /* Loop forever! */
	;
}

/*************************************************************************/
/*
 * Used to initialize a processes registers.
 */
void start_process(process_t *proc)
{
    memset(&proc->m_regs, 0, sizeof(struct regs));

    /* TODO: Actually load a process segment here! */
    proc->m_regs.gs = 0x10;
    proc->m_regs.fs = 0x10;
    proc->m_regs.es = 0x10;
    proc->m_regs.ds = 0x10;

    proc->m_regs.ss = read_ss();
    proc->m_regs.cs = read_cs();

    /* So the stub function can find the right proc. */
    proc->m_regs.eax = (uintptr_t)proc;

    proc->m_regs.useresp = read_esp();
    proc->m_regs.ebp = proc->m_regs.useresp;
    proc->m_regs.eflags = read_eflags();

    /* Give the the eip a pointer to our stub function. */
    proc->m_regs.eip = (uintptr_t)process_stub; 

    /* Primitive mutex, add to process list */
    cli();
    {
	proc->m_prev = 0;
	proc->m_next = s_firstProc;

	if (s_firstProc)
	    s_firstProc->m_prev = proc;
	else
	    s_lastProc = proc;

	s_firstProc = proc;
    }
    sti();
}

/*************************************************************************/

void create_process(process_entry_t entry)
{
    process_t *proc = kmalloc(sizeof(process_t));

    proc->m_state = TASK_CREATED;
    proc->m_entry = entry;
    start_process(proc);
}

/*************************************************************************/

void init_scheduler(void)
{
    
}

/*************************************************************************/

void run_scheduler(struct regs *r)
{
    process_t *i, *n;

    /* Clean up any finished processes here. */
    for (i = s_finished; i && (n = i->m_next, true); i = n)
	kfree(i);

    s_finished = NULL;

    /* Move to the next process in our list. */
    if (s_current != NULL)
    {
	if (s_current->m_slicesLeft > 0)
	{
	    --s_current->m_slicesLeft;
	    return;
	}
	
	/* Save the current processe's state. */
	memcpy(&s_current->m_regs, r, sizeof(struct regs));

	if (s_current->m_next)
	    s_current = s_current->m_next;
	else
	    s_current = s_firstProc;
    }
    else
	s_current = s_firstProc;

    if (s_current)
    {
	s_current->m_slicesLeft = 10;

	/* Our own stack pointer needs to remain the same. */
	s_current->m_regs.esp = r->esp;

	/* Load in the information to do context switch */
	memcpy(r, &s_current->m_regs, sizeof(struct regs));
    }
}

/*************************************************************************/
