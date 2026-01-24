/********************************************************************************************************************/

#include <stdio.h>

#include <kernel/arch/dconsole.h>
#include <kernel/debug.h>

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

    vsnprintf(buf, sizeof(buf), fmt, args);

    DebugConsole::PutString(buf, sizeof(buf));
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
