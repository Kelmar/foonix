/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>

//#include "bus.h"
#include "vga.h"
//#include "memory.h"
#include "cpu.h"

/********************************************************************************************************************/

namespace
{
    /************************************************************************************************************/
    /*
     * Constants
     */
    uint16_t *CRT_MEM = reinterpret_cast<uint16_t *>(0x000B8000);

    constexpr const uint16_t CRT_PORT = 0x03D4;
    
    constexpr const uint8_t CRT_CURSOR_HI = 0x0E;
    constexpr const uint8_t CRT_CURSOR_LO = 0x0F;

    constexpr const int CRT_WIDTH = 80;
    constexpr const int CRT_HEIGHT = 25;

    constexpr const size_t CRT_CELL_COUNT = CRT_WIDTH * CRT_HEIGHT;

    /************************************************************************************************************/
    /*
     * Local static variables
     */

    uint32_t s_cursorPos = 0;

    /************************************************************************************************************/
    /*
     * Utility functions
     */
    constexpr uint16_t MakeEntry(char chr, uint8_t color)
    {
        return (chr | (color << 8));
    }

    constexpr uint8_t MakeColor(uint8_t foreground, uint8_t background)
    {
        return (background & 0x0F) << 4 | (foreground & 0x0F);
    }

    constexpr uint8_t DEFAULT_COLOR = MakeColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    constexpr uint16_t BLANK_ENTRY = MakeEntry(' ', DEFAULT_COLOR);

    /************************************************************************************************************/

    inline
    uint8_t GetCRTRegister(int reg)
    {
        x86::outb(CRT_PORT, reg);
        return x86::inb(CRT_PORT + 1);
    }

    inline
    void SetCRTRegister(int reg, uint8_t value)
    {
        x86::outb(CRT_PORT, reg);
        x86::outb(CRT_PORT + 1, value);
    }

    /************************************************************************************************************/

    void ReadCursor()
    {
        s_cursorPos =
            GetCRTRegister(CRT_CURSOR_HI) << 8 |
            GetCRTRegister(CRT_CURSOR_LO);

        if (s_cursorPos >= CRT_CELL_COUNT)
            s_cursorPos = 0;
    }

    void UpdateCursor()
    {
        SetCRTRegister(CRT_CURSOR_LO, s_cursorPos & 0x0000'00FF);
        SetCRTRegister(CRT_CURSOR_HI, (s_cursorPos >> 8) & 0x0000'00FF);
    }

    inline int CursorRow()
    {
        return s_cursorPos / CRT_WIDTH;
    }

    inline int CursorCol()
    {
        return s_cursorPos % CRT_WIDTH;
    }

    /************************************************************************************************************/
    /**
     * @brief Scroll the screen by a number of lines.
     */
    static void Scroll(int lines)
    {
        if (lines == 0)
            return; // Nothing to do.

        int offset = CRT_WIDTH * lines;

        memcpy(CRT_MEM, CRT_MEM + offset, (CRT_CELL_COUNT - offset) << 1);

        // Fill remainder of screen with blanks
        uint16_t *blank = CRT_MEM + CRT_CELL_COUNT - offset;

        for (int i = 0; i < (lines * CRT_WIDTH); ++i)
            blank[i] = BLANK_ENTRY;
    }
}

/********************************************************************************************************************/

extern void keyboard_init();

/********************************************************************************************************************/

extern "C" void terminal_init(void)
{
    ReadCursor();
    UpdateCursor();

    keyboard_init();
}

/********************************************************************************************************************/
/*
 * Clears the terminal
 */
extern "C" void terminal_clear(void)
{
    s_cursorPos = 0;

    for (size_t i = 0; i < CRT_CELL_COUNT; ++i)
        CRT_MEM[i] = BLANK_ENTRY;

    UpdateCursor();
}

/********************************************************************************************************************/

extern "C" void terminal_putchar(char c)
{
    int r;

    switch (c)
    {
    case '\007': /* Bell */
        break;

    case 8:
    case 127: // Backspace
        if (s_cursorPos == 0)
            return;

        --s_cursorPos;
        CRT_MEM[s_cursorPos] = BLANK_ENTRY;
        break;

    case '\t': /* Tab */
        r = (s_cursorPos % CRT_WIDTH) & 7;
        r = r == 0 ? 8 : r;

        for (int i = 0; i < r; ++i)
            CRT_MEM[s_cursorPos + i] = BLANK_ENTRY;

        s_cursorPos += r;
        break;

    case '\r': /* CR */
        s_cursorPos = CursorRow() * CRT_WIDTH;
        break;

    case '\n': /* LF */
        s_cursorPos = (CursorRow() + 1) * CRT_WIDTH + CursorCol();
        break;

    default:
        CRT_MEM[s_cursorPos] = MakeEntry(c, DEFAULT_COLOR);
        ++s_cursorPos;
        break;
    }

    // Figure out how much we need to scroll by.
    for (r = 0; s_cursorPos >= CRT_CELL_COUNT; s_cursorPos -= CRT_WIDTH, ++r)
        ;

    Scroll(r);
    UpdateCursor();
}

/********************************************************************************************************************/

extern "C" void terminal_write(const char* data, size_t size)
{
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
