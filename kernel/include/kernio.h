/*************************************************************************/
/*
 * $Id: kernio.h 64 2010-09-03 03:34:41Z kfiresun $
 */
/*************************************************************************/

#ifndef _KERNIO_H_
#define _KERNIO_H_

/*************************************************************************/

#include "stdarg.h"

__BEGIN_DECLS

/*************************************************************************/
/* Low level display functions. */
/*************************************************************************/

void init_display(void);
void blank_screen(void);
void putstr(const char *str);

/*************************************************************************/
/* Higher level display functions. */
/*************************************************************************/

struct bitinfo_TYPE
{
    uint32_t bit;
    const char *name;
};

typedef struct bitinfo_TYPE bitinfo_t;

void kprint_bitinfo(bitinfo_t *info, uint32_t bits);

int vkprintf(const char *, va_list);
int kprintf(const char *, ...);
void hexdump(void *, size_t);

/// Dump a stack trace of the given stack ptr to the screen
void stack_trace(uintptr_t stack);

int get_debug_section(void);
void release_debug_section(int);
void update_debug_sections(void);
void set_debug_section_info(int, const char *, size_t);

/*************************************************************************/
/* Low level keyboard input. */
/*************************************************************************/

#define SS_SHIFT        0x0001  /* Shift key is pressed */
#define SS_CTRL         0x0002  /* Ctrl key is pressed */
#define SS_ALT          0x0004  /* Alt key is pressed */

/* These mirror the state of the keyboard lights */
#define SS_SCROLL       0x0100  /* Scroll lock is on */
#define SS_NUM          0x0200  /* Num lock is on */
#define SS_CAPS         0x0400  /* Caps lock is on */

typedef void (*key_callback_t)(uint8_t scancode, char charcode);

key_callback_t set_key_callback(key_callback_t cb);

void set_shift_state(uint16_t newstate);
uint16_t get_shift_state(void);

void init_keyboard(void);

/*************************************************************************/

__END_DECLS

#endif /* _KERNIO_H_ */

/*************************************************************************/
