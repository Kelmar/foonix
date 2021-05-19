/*************************************************************************/
/*
 * $Id: display.c 44 2010-02-19 23:53:25Z kfiresun $ 
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "stdlib.h"
#include "kernio.h"

/*************************************************************************/

#define VIDDET_LOCATION  (*((unsigned char *)0x0410))
#define VIDDET_MONO_FLAG 0x30

#define MISC_IO_READ ((unsigned char *)0x03CC)
#define MISC_IO_WRIT ((unsigned char *)0x03C2)

#define CUR_LOC_HI 0x0E
#define CUR_LOC_LO 0x0F

#define IO_ADDR_SEL 0x01

#define VGA_TEXT_LOC ((uint8_t *)0x000B8000)
#define MONO_TEXT_LOC ((uint8_t *)0x000B0000)

#define MAX_COLS  80
#define MAX_LINES 25
#define MAX_CHARS (MAX_COLS * MAX_LINES)

#define BLANK_CHAR 0x0720

/*************************************************************************/

#define TOF_INIT	0x01 /* Initialized flag */
#define TOF_ISMONO	0x02 /* Monocrome flag */

/*************************************************************************/

static unsigned char textout_flags = 0;

static unsigned char *crtr_addr = NULL;
static unsigned char *crtr_data = NULL;
static uint8_t *screen = NULL;

#define NUM_DEB_SECTIONS 8
#define SECT_LEN (MAX_COLS / NUM_DEB_SECTIONS)

/* "Debugging sections" */
static char *s_debug_sect[NUM_DEB_SECTIONS] = { NULL };

static char s_debug_info[MAX_COLS] = { 32 };

/* Set when scroll lock is on. */
//static bool_t s_userscroll = false;

/*************************************************************************/

int get_debug_section()
{
    int rval = 0;

    for (rval = 0; rval < NUM_DEB_SECTIONS; ++rval)
    {
	if (s_debug_sect[rval] == NULL)
	    break;
    }

    if (rval < NUM_DEB_SECTIONS)
	s_debug_sect[rval] = s_debug_info + (rval * SECT_LEN);
    else
	rval = -1;

    return rval;
}

/*************************************************************************/

void release_debug_section(int sect)
{
    if ((sect < 0) || (sect >= NUM_DEB_SECTIONS))
	return;

    s_debug_sect[sect] = NULL;
}

/*************************************************************************/
/*
 * This is a kernel thread that never exits.
 */
void update_debug_sections(void)
{
    int i;

    for (;;)
    {
	for (i = 0; i < MAX_COLS; ++i)
	{
	    screen[i * 2 + 0] = s_debug_info[i];
	    screen[i * 2 + 1] = 0x7F; /* Bright white on grey */
	}

	i = get_shift_state();
	if (i & SS_SCROLL)
	    screen[158] = 23; /* goffy arrow */
	else
	    screen[158] = ' ';

	if (i & SS_NUM)
	    screen[156] = '#';
	else
	    screen[156] = ' ';

	if (i & SS_CAPS)
	    screen[154] = 'A';
	else
	    screen[154] = ' ';

	/* Bright green on grey */
	screen[155] = 0x7A;
	screen[157] = 0x7A;
	screen[159] = 0x7A;
    }
}

/*************************************************************************/

void set_debug_section_info(int sect, const char *info, size_t ilen)
{
    if ((sect < 0) || (sect >= NUM_DEB_SECTIONS))
	return;

    memset(s_debug_sect[sect], 0, SECT_LEN);
    strkcat(s_debug_sect[sect], SECT_LEN, info, ilen);
}

/*************************************************************************/

static void init_mono(void)
{
    textout_flags |= TOF_ISMONO;
    screen = MONO_TEXT_LOC;

    /*
     * I hope this is correct, but if I recall correctly, the reason for
     * the register address detection in the init_vga() function is to
     * allow for two video cards (one mono, and one vga) in older systems.
     *
     * It is doubtful this is even really a problem any longer as I don't
     * even think that monochrome display cards are even made any longer.
     */
    crtr_addr = (unsigned char *)0x03B4;
    crtr_data = (unsigned char *)0x03B5;
}

/*************************************************************************/

static void init_vga(void)
{
    uint8_t ioreg = 0;
    uint32_t crt_base = 0x03B4;

    textout_flags &= ~TOF_ISMONO;
    screen = VGA_TEXT_LOC;

    /* Get the location of the CRT registers. */
    ioreg = bus_read_1(MISC_IO_READ, 0);

    if (ioreg & IO_ADDR_SEL)
	crt_base += 0x0020;

    crtr_addr = (unsigned char *)crt_base;
    crtr_data = (unsigned char *)(crt_base + 1);
}

/*************************************************************************/
/*
 * Initializes the display functions.
 */
void init_display(void)
{
    if (textout_flags & TOF_INIT)
	return; /* Already initialized. */

    if ((VIDDET_LOCATION & VIDDET_MONO_FLAG) == VIDDET_MONO_FLAG)
	init_mono();
    else
	init_vga();

    textout_flags |= TOF_INIT;

    kprintf("Display initialized\n");
}

/*************************************************************************/
/*
 * Get's the current cursor location.
 */
uint16_t get_cursor_loc()
{
    uint8_t b[2];

    bus_write_1(crtr_addr, 0, CUR_LOC_LO);
    b[0] = bus_read_1(crtr_data, 0);

    bus_write_1(crtr_addr, 0, CUR_LOC_HI);
    b[1] = bus_read_1(crtr_data, 0);

    return ((uint16_t)(b[1]) << 8 | b[0]);
}
/*************************************************************************/
/*
 * Sets the current cursor location.
 */
void set_cursor_loc(uint8_t col, uint8_t row)
{
    uint16_t tmp = row * MAX_COLS + col;

    bus_write_1(crtr_addr, 0, CUR_LOC_HI);
    bus_write_1(crtr_data, 0, (uint8_t)((tmp >> 8) & 0xFF));

    bus_write_1(crtr_addr, 0, CUR_LOC_LO);
    bus_write_1(crtr_data, 0, (uint8_t)(tmp & 0xFF));
}

/*************************************************************************/
/*
 * Scrolls the screen upwards one row.
 */
static void scroll_up(void)
{
    const unsigned int last_line = (MAX_LINES - 1) * MAX_COLS;

    memcpy(screen, screen + MAX_COLS * 2, last_line * 2);

    /* Fill with white on black spaces at the bottom. */
    _memsetw(screen + last_line * 2, BLANK_CHAR, MAX_COLS);
}

/*************************************************************************/
/*
 * Blanks the screen and sets the cursor location to the top left.
 */
void blank_screen(void)
{
    /* Fill with white on black spaces. */
    _memsetw(screen, BLANK_CHAR, MAX_CHARS);

    /* Put the cursor at the start of the screen. */
    set_cursor_loc(0, 0);
}

/*************************************************************************/
/*
 * Write a raw NUL terminated string at the current cursor location.
 *
 * Scrolls the screen and advances the cursor.
 */
void putstr(const char *str)
{
    int line, col;
    uint16_t o;
    int i;

    o = get_cursor_loc();
    line = o / MAX_COLS;
    col  = o % MAX_COLS;

    for (i = 0; str[i] != '\0'; ++i)
    {
	if (str[i] < ' ')
	{
	    /* Process a special character. */

	    switch (str[i])
	    {
	    case '\007': /* Bell */
		break;

	    case '\t':   /* Tab */
		o = col & 7;
		col += (o == 0) ? 8 : o;
		break;

	    case '\r': /* CR */
	    case '\n': /* LF */
		/* Move to begining of next line. */
		col = 0;
		++line;

		/* TODO: Check for \r\n pair? */
		break;

	    default:
		/* Ignore non-printable characters. */
		break;
	    }
	}
	else
	{
	    o = (line * MAX_COLS + col) * 2;
	    screen[o + 0] = str[i];
	    screen[o + 1] = 0x07; /* white */

	    ++col;
	}

	/*
	 * Get the cursor location into a correct "state".
	 */
	while (col >= MAX_COLS)
	{
	    col -= MAX_COLS;
	    ++line;
	}

	while (line >= MAX_LINES)
	{
	    --line;
	    scroll_up();
	}
    }

    set_cursor_loc(col, line);
}

/*************************************************************************/
