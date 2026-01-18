/********************************************************************************************************************/

#ifndef __KERNEL_DEBUG_CONSOLE_H__
#define __KERNEL_DEBUG_CONSOLE_H__

/********************************************************************************************************************/
/*
 * Sense the debugging console might get used before we have a proper MMU setup, we really just use a namespace
 * instead of a class.
 */

#include <stdint.h>
#include <stddef.h>

namespace DebugConsole
{
    /**
     * @brief Initialization called before MMU setup.
     */
    int Init1(void);

    /**
     * @brief Called after MMU setup and the IDT is loaded.
     */
    int Init2(void);

    /**
     * @brief Reads a single character from the console.
     * 
     * @remark Does not block, if no character is waiting, returns -1
     */
    int ReadChar(void);

    /**
     * @brief Sends a single character to the console.
     */
    int PutChar(char c);

    /**
     * @brief Writes data to the console.
     * 
     * @remark This writes raw data to the console and does not try
     * to interperet the data (including NULL characters).
     */
    void Write(const char *str, size_t len);

    /**
     * @brief Writes a null terminated string to the console.
     */
    // TODO: Make this use our internal string class?
    void PutString(const char *str, size_t len);

    /**
     * @brief Dump a 64-bit unsigned int to the console.
     * 
     * This is termporary until we get printf() working with 64-bit integers
     */
    void DumpUInt64Hex(uint64_t i);
}

/********************************************************************************************************************/

#endif /* __KERNEL_DEBUG_CONSOLE_H__ */

/********************************************************************************************************************/
