/********************************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <string_view>

#include <kernel/arch/dconsole.h>
#include <kernel/kernel_args.h>
#include <kernel/console.h>
#include <kernel/debug.h>
#include <kernel/utilities.h>

#include <kernel/vm.h>

#include <kernel/kalloc.h>

#include "paging.h"

/********************************************************************************************************************/

namespace
{
    void VarsCommand(size_t, const std::string_view[])
    {
        console
            << "kernel_start: 0x" << hex(kernel::arguments.KernelCode.Base, -8)
            << " aligned: " << hex(kernel::arguments.KernelCode.BaseAligned(), -8)
            << "\r\n";

        console
            << "kernel_end: 0x" << hex(kernel::arguments.KernelCode.End(), -8)
            << " aligned: " << hex(kernel::arguments.KernelCode.EndAligned(), -8)
            << "\r\n";

        console
            << "heap_start: 0x" << hex(kernel::arguments.HeapStart, -8) << "\r\n";
    }

    void DumpCommand(size_t argCount, const std::string_view args[])
    {
        if (argCount < 3)
        {
            console
                << "Not enough arguments." << "\r\n"
                << "dump <start> <count>\r\n";

            return;
        }

        auto result = util::parseInt(args[1]);

        if (!result)
        {
            console << "Invalid start: " << args[1] << "\r\n";
            return;
        }

        paddr_t start = result.value();

        result = util::parseInt(args[2]);

        if (!result)
        {
            console << "Invalid length: " << args[2] << "\r\n";
            return;
        }

        size_t length = result.value();

        if (length == 0)
        {
            console << "Invalid length: " << args[2] << "\r\n";
            return;
        }

        console.set_paged(true);
        console << "Memory Dump 0x" << hex(start, -8) << "-0x" << hex(start + length, -8) << "\r\n";

        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(start);

        char buf[17];
        int mod;

        buf[16] = '\0';

        for (size_t i = 0; i < length; ++i, ++ptr)
        {
            mod = i & 15;

            if (mod == 0)
            {
                if (i > 0)
                    console << "| " << buf << "\r\n";

                console << hex(start + i, -8) << " | ";
            }
            else if (mod == 8)
                console << "- ";

            uint8_t val = *ptr;
            buf[mod] = ((val >= 32) && (val < 127)) ? val : '.';

            console << hex(val, -2) << " ";
        }

        if (mod != 0)
        {
            buf[mod + 1] = '\0';
            console << "| " << buf << "\r\n";
        }

        console.set_paged(false);
    }

    void TestAllocCommand(size_t argCount, const std::string_view args[])
    {
        if (argCount < 2)
        {
            console
                << "Not enough arguments." << "\r\n"
                << "testa <size>\r\n";

            return;
        }

        auto result = util::parseInt(args[1]);

        if (!result)
        {
            console << "Invalid length: " << args[1] << "\r\n";
            return;
        }

        size_t sz = result.value();

        if (sz == 0)
        {
            console << "Cannot allocate less than 1 byte!\r\n";
            return;
        }
        if (sz >= cpu::PageSize)
        {
            console << "Cannot allocate more than " << cpu::PageSize << " bytes!\r\n";
            return;
        }

        console << "Attempting to allocate " << sz << " bytes.";

        void *ptr = kalloc(sz);
        uintptr_t p = reinterpret_cast<uintptr_t>(ptr);

        console << "\r\nGot pointer: " << hex(p, -8) << "\r\nFreeing...";

        kfree(ptr);

        console << "  freed?  Maybe?  Let's hope!\r\n";
    }

    void FaultCommand(size_t, const std::string_view [])
    {
        console << "Causing a page fault.\r\n";

        char *data = reinterpret_cast<char *>(0x0080'0000);
        char c = *data; // Fault should happen here.
        (void)c;
    }
}

/********************************************************************************************************************/

namespace
{
    constexpr const size_t ARGS_SZ = 16;

    char s_debugCmd[512];

    std::string_view s_debugArgs[ARGS_SZ];

    int s_debugArgCount = 0;

    typedef void (*DebugFn)(size_t argCount, const std::string_view args[]);

    struct DebugCommand
    {
        const char *text;
        DebugFn callback;
    };

    /// @brief Static null terminated list of commands.
    DebugCommand s_commands[] =
    {
        { "dump"   , DumpCommand         },
        { "meminfo", vmm::MemInfoCommand },
        { "vars"   , VarsCommand         },
        { "fault"  , FaultCommand        },
        { "testa"  , TestAllocCommand    },
        { 0        , 0                   } // terminator
    };

    /************************************************************************************************************/
    /**
     * @brief Dispatch the command found in s_debugArgs[0].
     *
     * If there are no arguments in the s_debugArgs list (i.e. no command... (i.i.e. an empty line)), then the
     * function does nothing.
     */
    void dispatch()
    {
        if (s_debugArgCount == 0)
            return;

        for (size_t i = 0; s_commands[i].text; ++i)
        {
            int res = s_debugArgs[0].compare(s_commands[i].text);

            if (res == 0)
            {
                s_commands[i].callback(s_debugArgCount, s_debugArgs);
                return;
            }
        }

        console << "Unknown command: " << s_debugArgs[0] << "\r\n";
    }

    /************************************************************************************************************/
    /**
     * @brief Derpy debug shell parser.
     *
     * A simple parser that splits along white spaces.  The arguments will be trimmed of leading and trailing
     * white space in the g_debugArgs list.
     *
     * The first "argument" is the command itself.
     *
     * This function does not handle any complex parsing; it doesn't deal with quotes or other such things.
     */
    void parse()
    {
        bool inArg = false;

        s_debugArgCount = 0;

        for (int i = 0; i < ARGS_SZ; ++i)
            s_debugArgs[i] = std::string_view();

        size_t i, start = 0;

        for (i = 0; i < (sizeof(s_debugCmd) - 1) && (s_debugArgCount < ARGS_SZ); ++i)
        {
            char c = s_debugCmd[i];

            if (c == '\0')
                break;

            if (isspace(c))
            {
                if (inArg)
                {
                    // First space after non-space character.

                    s_debugArgs[s_debugArgCount] = std::string_view(s_debugCmd + start, i - start);
                    ++s_debugArgCount;
                    inArg = false;
                }

                continue;
            }

            if (!inArg)
            {
                // First non-space character after space.
                start = i;
                inArg = true;
            }
        }

        if (inArg)
        {
            // End of string, add to list
            s_debugArgs[s_debugArgCount] = std::string_view(s_debugCmd + start, i - start);
            ++s_debugArgCount;
        }
    }
}

/********************************************************************************************************************/

void Debug::PrintF(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vPrintF(fmt, args);
    va_end(args);
}

/********************************************************************************************************************/

void Debug::vPrintF(const char *fmt, va_list args)
{
    char buf[512];

    int len = vsnprintf(buf, sizeof(buf), fmt, args);

    DebugConsole::PutString(buf, len);
}

/********************************************************************************************************************/

void Debug::shell()
{
    console << "Welcome to kernel debug land.\r\n";

    while (true)
    {
        console << "\r\nkdbg> ";

        if (!console.get_line(s_debugCmd, sizeof(s_debugCmd)))
            continue;

        parse();
        dispatch();
    }
}

/********************************************************************************************************************/
/// @brief Display a "STOP ERROR" message and halt the system.
extern "C"
void kpanic(const char *message)
{
    Debug::PrintF(
        "**********************************\n"
        "STOP ERROR: %s\n"
        "**********************************\n"
        "\nSYSTEM HALTED!",
        message
    );

    khalt();
}

/********************************************************************************************************************/

/// @brief Display a "STOP ERROR" message and halt the system.
extern "C"
void kassert(const char *test, const char *reason, int line, const char *file, const char *function)
{
    Debug::PrintF(
        "**********************************\n"
        "STOP ERROR: %s\n"
        "TEST: %s\n"
        "LOCATOIN: %s (%s:%d)\n"
        "**********************************\n"
        "\nSYSTEM HALTED!",
        reason, test, function, file, line
    );

    khalt();
}

/********************************************************************************************************************/
