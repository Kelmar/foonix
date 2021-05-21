/*************************************************************************/
/*
 * $Id: keyboard.c 180 2011-08-10 03:04:04Z kfiresun $ 
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "stdlib.h"
#include "kernio.h"
#include "scan.h"

/*************************************************************************/

#define KEY_PORT ((io_port_t)0x0060)

/*************************************************************************/
/*
 * This is a basic US ASCII keymapping.
 */
char ascii_keymap[128] =
{
    0,
    27, /* Escape */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 
    8 /* Backspace */,
    9 /* TAB */,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13 /* Enter */,
    0 /* control */,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0 /* left shift */,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0 /* right shift */,
    '*',
    0 /* alt */,
    ' ' /* space bar */,
    0 /* caps lock */,
    0 /* F1 key ... > */,
    0, 0, 0, 0, 0, 0, 0, 0,
    0 /* < ... F10 */,
    0 /* num lock */,
    0 /* scroll lock */,
    0 /* home */,
    0 /* up arrow */,
    0 /* page up */,
    '-',
    0 /* left arrow */,
    0,
    0 /* right arrow */,
    '+',
    0 /* end */,
    0 /* down arrow */,
    0 /* page down */,
    0 /* insert */,
    0 /* delete */,
    0, 0, 0,
    0 /* F11 */,
    0 /* F12 */,
    0 /* all other keys undefined. */
};

uint8_t last_states[127] = { 0 };

/*************************************************************************/

#define SS_SHIFT	0x0001	/* Shift key is pressed */
#define SS_CTRL		0x0002	/* Ctrl key is pressed */
#define SS_ALT		0x0004	/* Alt key is pressed */

/* These mirror the state of the keyboard lights */
#define	SS_SCROLL	0x0100	/* Scroll lock is on */
#define SS_NUM		0x0200	/* Num lock is on */
#define SS_CAPS		0x0400	/* Caps lock is on */

/* 
 * The state of the various shift keys.
 */
static uint16_t s_shift_state = 0;

static key_callback_t s_callback = NULL;

/*************************************************************************/

static void keyboard_handler(struct regs *r);

/*************************************************************************/

key_callback_t set_key_callback(key_callback_t cb)
{
    key_callback_t rval = s_callback;
    s_callback = cb;
    return rval;
}

/*************************************************************************/

void kbd_wait_state(int bit, bool_t set)
{
    if (set)
    {
        for (;;)
        {
            uint8_t state = bus_read_1(KEY_PORT, 4);

            if ((state & bit) != 0)
                break;
        }
    }
    else
    {
        for (;;)
        {
            uint8_t state = bus_read_1(KEY_PORT, 4);

            if ((state & bit) == 0)
                break;
        }
    }
}

/*************************************************************************/

static void toggle_lock_state(int shift_bit)
{
    uint8_t lstate;

    s_shift_state ^= shift_bit;

    lstate = (s_shift_state & 0xFF00) >> 8;

    kbd_wait_state(2, false);
    bus_write_1(KEY_PORT, 0, 0xED);

    kbd_wait_state(2, false);
    bus_write_1(KEY_PORT, 0, lstate);
}

/*************************************************************************/
/*
 * Get's the state of the keyboard's various lock key lights, and updates
 * the s_shift_state variable as needed.
 */
static void read_lock_state(void)
{
    uint8_t lstate;

    bus_write_1(KEY_PORT, 0, 0xED);
    soft_delay();
    lstate = bus_read_1(KEY_PORT, 0);

    s_shift_state &= 0x00FF;
    s_shift_state |= (lstate << 8);
}

/*************************************************************************/

void set_shift_state(uint16_t newstate)
{
    /*uint8_t lstate;*/

    newstate &= 0xFF00; /* Can't set state of Shift, Ctrl, and Alt keys. */

    s_shift_state = newstate | (s_shift_state & 0x00FF);

#if 0
    /* Set the keyboard lights */
    lstate = (newstate >> 8) & 0xFF;

    kbd_wait_state(2);

    bus_write_1(KEY_PORT, 0, 0xED);
    soft_delay();
    bus_write_1(KEY_PORT, 0, lstate);
#endif
}

/*************************************************************************/

uint16_t get_shift_state(void)
{
    return s_shift_state;
}

/*************************************************************************/
/*
 * Initializes the keyboard.
 */
void init_keyboard(void)
{
    read_lock_state();

    set_isr_callback(IRQISR(1), keyboard_handler);

    kprintf("Keyboard initialized\n");
}

/*************************************************************************/

uint8_t read_scancode(void)
{
    kbd_wait_state(1, true);

    return bus_read_1(KEY_PORT, 0);
}

/*************************************************************************/

static void keyboard_handler(struct regs *r)
{
    bool_t do_callback = false;  /* Assume no callback call */

    uint8_t shift_adj = 0;
    int lock_bit = 0;
    uint8_t scancode;
    bool_t pressed;

    UNUSED(r);
    scancode = read_scancode();

    kprintf("Scan code: 0x%02X", scancode);

    pressed = (scancode & 0x80) == 0;
    scancode &= 0x7F;

    if (pressed)
    {
        if (last_states[scancode] < 127)
            ++last_states[scancode]; // Increment for repeat
    }
    else
        last_states[scancode] = 0;

    kprintf("(%d)\n", last_states[scancode]);

    switch (scancode)
    {
    case SC_L_SHIFT:
    case SC_R_SHIFT:
        shift_adj = SS_SHIFT;
        break;

    case SC_L_CTRL:
/*    case SC_R_CTRL: */
        shift_adj = SS_CTRL;
        break;

    case SC_L_ALT:
/*    case SC_R_ALT: */
        shift_adj = ~SS_ALT;
        break;

    case SC_CAPS_LOCK:
        lock_bit = SS_CAPS;
        break;

    case SC_NUM_LOCK:
        lock_bit = SS_NUM;
        break;

    case SC_SCROLL_LOCK:
        lock_bit = SS_SCROLL;
        break;

    default:
        do_callback = true;
        break;
    }

    if (lock_bit && (last_states[scancode] == 1))
    {
        toggle_lock_state(lock_bit);
    }

    if (pressed)
	s_shift_state |= shift_adj;
    else
	s_shift_state &= ~shift_adj;

    if (do_callback && (s_callback != NULL))
    {
        char charcode = ascii_keymap[scancode];

        s_callback(scancode, charcode);
    }
}

/*************************************************************************/
