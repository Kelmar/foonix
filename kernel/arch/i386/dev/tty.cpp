#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/tty.h>

#include "bus.h"
#include "vga.h"
#include "memory.h"

/*************************************************************************/

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

/*
static uint8_t* const MISC_IO_READ = (uint8_t*)0x03CC;
static uint8_t* const MISC_IO_WRIT = (uint8_t*)0x03C2;

static const uint32_t CRT_BASE = 0x03B4;

static const uint8_t CUR_LOC_HI = 0x0E;
static const uint8_t CUR_LOC_LO = 0x0F;

static const uint8_t IO_ADDR_SEL = 0x01;
*/

/*************************************************************************/
 
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

//static uint8_t* crtr_addr;
//static uint8_t* crtr_data;

/*************************************************************************/

static void set_loc(int col, int row);
static void scroll_up(void);

/*************************************************************************/

extern "C" void terminal_init(void)
{
#if 0
    uint8_t ioreg = 0;

    /* Get the location of the CRT registers. */
    ioreg = bus_read_1(MISC_IO_READ, 0);

    if (ioreg & IO_ADDR_SEL)
        crt_base += 0x0020;

    crtr_addr = (unsigned char *)crt_base;
    crtr_data = (unsigned char *)(crt_base + 1);
#endif

    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;

    terminal_clear();
    set_loc(0, 0);

    printf("Terminal initialized\n");
}

/*************************************************************************/
/*
 * Clears the terminal
 */
extern "C" void terminal_clear(void)
{
    const uint16_t entry = vga_entry(' ', terminal_color);

    _memsetw(terminal_buffer, entry, VGA_WIDTH * VGA_HEIGHT);

    set_loc(0, 0);
}

/*************************************************************************/
/*
 * Sets the cursor's current location
 */
static void set_loc(int col, int row)
{
#if 0
    uint16_t tmp = row * VGA_WIDTH * col;

    bus_write_1(crtr_addr, 0, CUR_LOC_HI);
    bus_write_1(crtr_data, 0, (uint8_t)((tmp >> 8) & 0xFF));

    bus_write_1(crtr_addr, 0, CUR_LOC_LO);
    bus_write_1(crtr_data, 0, (uint8_t)(tmp & 0xFF));
#endif

    terminal_column = col;
    terminal_row = row;
}

/*************************************************************************/
#if 0
/*
 * Get's the current cursor location.
 * (For reference)
 */
static uint16_t get_loc()
{
    uint8_t b[2];

    bus_write_1(crtr_addr, 0, CUR_LOC_LO);
    b[0] = bus_read_1(crtr_data, 0);

    bus_write_1(crtr_addr, 0, CUR_LOC_HI);
    b[1] = bus_read_1(crtr_data, 0);

    return ((uint16_t)(b[1]) << 8 | b[0]);
}
#endif

/*************************************************************************/
/*
 * Scrolls the screen upwards one row.
 */
static void scroll_up(void)
{
    const unsigned int last_line = (VGA_HEIGHT - 1) * VGA_WIDTH;
    const uint16_t entry = vga_entry(' ', terminal_color);

    memcpy(terminal_buffer, terminal_buffer + VGA_WIDTH, last_line * 2);

    /* Fill with term color spaces at the bottom. */
    _memsetw(terminal_buffer + last_line, entry, VGA_WIDTH);
}

/*************************************************************************/

extern "C" void terminal_putchar(char c)
{
    size_t col = terminal_column;
    size_t row = terminal_row;
    int o;

    switch (c)
    {
    case '\007': /* Bell */
        break;

    case '\t': /* Tab */
        o = col & 7;
        o = o == 0 ? 8 : o;

        for (int i = 0; i < o; ++i)
            terminal_buffer[col + row * VGA_WIDTH] = vga_entry(' ', terminal_color);

        col += o;

        break;

    case '\r': /* CR */
    case '\n': /* LF */
        ++row;
        col = 0;
        break;

    default:
        if (c >= ' ')
            terminal_buffer[col + row * VGA_WIDTH] = vga_entry(c, terminal_color);

        ++col;
    }

    if (col >= VGA_WIDTH)
    {
        col -= VGA_WIDTH;
        ++terminal_row;

    }

    while (row >= VGA_HEIGHT)
    {
        scroll_up();
        --row;
    }

    set_loc(col, row);
}

/*************************************************************************/

extern "C" void terminal_write(const char* data, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        terminal_putchar(data[i]);
}

/*************************************************************************/

extern "C" void terminal_writestr(const char* str)
{
    terminal_write(str, strlen(str));
}

/*************************************************************************/
