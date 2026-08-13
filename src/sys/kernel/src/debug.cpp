/********************************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <kernel/arch/dconsole.h>
#include <kernel/console.h>
#include <kernel/debug.h>

/********************************************************************************************************************/

namespace
{
    void TestCommand(const util::span<char *> &args)
    {
        console::putstr("test command\r\n");
        console::putstr("Args:\r\n");

        for (auto arg : args)
        {
            console::putstr("  ");
            console::putstr(arg);
            console::putstr("\r\n");
        }
    }
}

/********************************************************************************************************************/

namespace
{
    constexpr const size_t ARGS_SZ = 16;

    char s_debugCmd[512];

    char *s_debugArgs[ARGS_SZ];

    int s_debugArgCount = 0;

    typedef void (*DebugFn)(const util::span<char *> &);

    struct DebugCommand
    {
        const char *text;
        DebugFn callback;
    };

    DebugCommand s_commands[] =
    {
        { "test", TestCommand },
        { 0, 0 }
    };

    /************************************************************************************************************/

    void dispatch()
    {
        if (s_debugArgCount == 0)
            return;

        for (size_t i = 0; s_commands[i].text; ++i)
        {
            int res = strncmp(s_debugArgs[0], s_commands[i].text, sizeof(s_debugCmd));

            if (res == 0)
            {
                util::span<char *> args(s_debugArgs, s_debugArgCount);
                s_commands[i].callback(args);
                break;
            }
        }
    }

    /************************************************************************************************************/

    void parse()
    {
        bool inArg = false;

        s_debugArgCount = 0;

        for (int i = 0; i < ARGS_SZ; ++i)
            s_debugArgs[i] = nullptr;

        size_t i;

        for (i = 0; i < (sizeof(s_debugCmd) - 1) && (s_debugArgCount < ARGS_SZ); ++i)
        {
            char c = s_debugCmd[i];

            if (c == '\0')
                break;

            if (isspace(c))
            {
                if (inArg)
                {
                    s_debugCmd[i] = '\0';
                    ++s_debugArgCount;
                    inArg = false;
                }

                continue;
            }

            if (!inArg)
            {
                s_debugArgs[s_debugArgCount] = &s_debugCmd[i];
                inArg = true;
            }
        }

        if (inArg)
            ++s_debugArgCount;

        s_debugCmd[i] = '\0';
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
    console::putstr("Welcome to kernel debug land.\r\n");

    while (true)
    {
        console::putstr("\nkdbg> ");

        if (!console::get_line(s_debugCmd, sizeof(s_debugCmd)))
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
