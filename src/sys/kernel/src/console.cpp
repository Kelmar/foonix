/********************************************************************************************************************/
/********************************************************************************************************************/

#include <stdint.h>

#include <kernel/console.h>
#include <kernel/tty.h>

#include "cpu.h"

/********************************************************************************************************************/

Console console;

/********************************************************************************************************************/

void Console::pause_check()
{
    if (!m_paged)
        return;

    if (++m_lineCount < 20)
        return;

    m_lineCount = 0;
    terminal_writestr("PAUSED\n");

    while (getchar() <= 0)
        cpu::pause();
}

/********************************************************************************************************************/

void Console::init()
{
    terminal_init();
}

/********************************************************************************************************************/

bool Console::set_paged(bool value)
{
    bool res = m_paged;

    m_paged = value;

    if (m_paged)
        m_lineCount = 0;

    return res;
}

/********************************************************************************************************************/

void Console::putchar(char c)
{
    terminal_putchar(c);

    if ((c == '\r') || (c == '\n'))
        pause_check();
}

/********************************************************************************************************************/

void Console::putstr(const char *str)
{
    terminal_writestr(str);
}

/********************************************************************************************************************/

int Console::getchar()
{
    return terminal_getchar();
}

/********************************************************************************************************************/

size_t Console::get_line(util::span<char> &buf)
{
    const size_t max = buf.size() - 1;

    if (buf.empty() || (max == 0))
        return 0; // No space!

    size_t idx = 0;
    
    while (true)
    {
        int c = getchar();

        switch (c)
        {
        case -1:
        case 0:
            cpu::pause();
            break;

        case 8:
        case 127: // Backspace
            if (idx == 0)
            {
                terminal_putchar('\a');
                break;
            }
            
            buf[idx] = '\0';
            --idx;

            terminal_putchar(c);
            break;

        case 27: // Escape
            while (idx > 0)
            {
                terminal_putchar('\b');
                --idx;
            }
            break;

        case '\r':
        case '\n': // Enter
            if (idx > 0)
            {
                terminal_writestr("\r\n");
                
                buf[idx] = '\0';
                return idx;
            }
            break; // ignore leading \r or \n

        default:
            buf[idx] = c;

            if (++idx >= max)
            {
                terminal_putchar('\a'); // Alert user
                break;
            }

            terminal_putchar(c);
            break;
        }
    }
}

/********************************************************************************************************************/
