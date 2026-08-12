/********************************************************************************************************************/

#ifndef __FONIX_KERNEL_CONSOLE_H__
#define __FONIX_KERNEL_CONSOLE_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <kernel/span.h>

/********************************************************************************************************************/

namespace console
{
    void init();
    bool set_paged(bool value);

    void putchar(char c);
    int getchar();
        
    size_t get_line(util::span<char> &buf);

    inline
    size_t get_line(char *buf, size_t size)
    {
        util::span<char> data(buf, size);
        return get_line(data);
    }
}

/********************************************************************************************************************/

#endif /* __FONIX_KERNEL_CONSOLE_H__ */

/********************************************************************************************************************/
