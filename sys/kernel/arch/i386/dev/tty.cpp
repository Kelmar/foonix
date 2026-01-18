/********************************************************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/tty.h>

#include "bus.h"
#include "vga.h"
#include "memory.h"

/********************************************************************************************************************/

#define vga_entry(c__, clr__) ((uint16_t)c__ | (uint16_t)(clr__ << 8))

namespace
{
    /************************************************************************************************************/

    const size_t VGA_WIDTH = 80;
    const size_t VGA_HEIGHT = 25;
    //uint16_t* const VGA_MEMORY = (uint16_t*)0x000B8000;

    const uint8_t CUR_LOC_HI = 0x0E;
    const uint8_t CUR_LOC_LO = 0x0F;

    /*const uintptr_t CRT_BASE = 0x03B4;
    
    void* const MISC_IO_READ = (void*)0x03CC;
    void* const MISC_IO_WRIT = (void*)0x03C2;

    const uint8_t IO_ADDR_SEL = 0x01;*/

    /************************************************************************************************************/

    size_t terminal_row;
    size_t terminal_column;
    uint8_t terminal_color;
    uint16_t* terminal_buffer;

    bus* crt_bus;

    //uint8_t* crtr_addr;
    //uint8_t* crtr_data;

    /************************************************************************************************************/
    /*
     * Sets the cursor's current location
     */
    static void set_loc(int col, int row)
    {
        if (crt_bus)
        {
            uint16_t tmp = row * VGA_WIDTH * col;

            crt_bus->byte(0, CUR_LOC_HI);
            crt_bus->byte(1, (uint8_t)((tmp >> 8) & 0xFF));

            crt_bus->byte(0, CUR_LOC_LO);
            crt_bus->byte(1, (uint8_t)(tmp & 0xFF));
        }

        terminal_column = col;
        terminal_row = row;
    }

    /************************************************************************************************************/
#if 0
/*
 * Get's the current cursor location.
 * (For reference)
 */
    static uint16_t get_loc()
    {
        uint8_t b[2];

        crt_bus->byte(0, CUR_LOC_LO);
        b[0] = crt_bus->byte(1);

        crt_bus->byte(0, CUR_LOC_HI);
        b[1] = crt_bus->byte(1);

        return ((uint16_t)(b[1]) << 8 | b[0]);
    }
#endif

    /************************************************************************************************************/
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
}

/********************************************************************************************************************/

void terminal_pre_init(void)
{
    crt_bus = nullptr;

    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = ((uint16_t*)0x000B8000);

    terminal_clear();

    terminal_buffer[0] = vga_entry('C', vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

/********************************************************************************************************************/

extern "C" void terminal_init(void)
{
#if 0
    bus misc_io(MISC_IO_READ, IO_ADDR_SEL);

    uint8_t ioreg = misc_io.read1(0);

    uintptr_t crt_base = CRT_BASE;

    if (ioreg & IO_ADDR_SEL)
        crt_base += 0x0020;

    crt_bus = new bus((io_port_t)crt_base, 2);
#endif
}

/********************************************************************************************************************/
/*
 * Clears the terminal
 */
extern "C" void terminal_clear(void)
{
    //const uint16_t entry = vga_entry(' ', terminal_color);
    //(void)entry;

    //_memsetw(terminal_buffer, entry, VGA_WIDTH * VGA_HEIGHT);

    //set_loc(0, 0);

    terminal_buffer[0] = vga_entry('B', vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

/********************************************************************************************************************/

void terminal_dump(size_t v)
{
    const char *hex = "0123456789ABCDEF";

    int idx = 0;

    do
    {
        char c = hex[(v & 0x0F)];
        terminal_buffer[idx] = vga_entry(c, 0x07);
        v >>= 4;
    } while (v > 0);
}

/********************************************************************************************************************/

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

/********************************************************************************************************************/

extern "C" void terminal_write(const char* data, size_t size)
{
    /*
    size_t i = 0;
    while (i < size)
    {
        terminal_putchar(data[i]);
        ++i;
    }
    */

    for (size_t i = 0; i < size; ++i)
        terminal_putchar(data[i]);
}

/********************************************************************************************************************/

extern "C" void terminal_writestr(const char* str)
{
    terminal_write(str, strlen(str));
}

/********************************************************************************************************************/

extern "C" void terminal_write64(uint64_t val)
{
    // We write our string backwards.
    char nbuf[17];
    int d;
        
    nbuf[16] = '\0';
    
    for (int i = 15; i >= 0; --i)
    {
        d = val & 0x0F;
        val >>= 4;

        nbuf[i] = (char)((d >= 10) ? (d + 55) : (d + '0'));
    };

    terminal_writestr("0x");
    terminal_writestr(nbuf);
}

/********************************************************************************************************************/
