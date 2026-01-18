/********************************************************************************************************************/

#ifndef __FONIX_KERNEL_DEBUG_H__
#define __FONIX_KERNEL_DEBUG_H__

/********************************************************************************************************************/

#include <stdarg.h>

namespace Debug
{
    /**
     * @brief Formatted printing to debug console.
     */
    void PrintF(const char *fmt, ...);

    /**
     * @brief Formatted printing to debug console.
     */
    void vPrintF(const char *fmt, va_list args);

    /**
     * @brief Halts the kernel with a message.
     */
    __attribute__((__noreturn__))
    void Panic(const char *msg);
}

/********************************************************************************************************************/

#endif /* __FONIX_KERNEL_DEBUG_H__ */

/********************************************************************************************************************/
