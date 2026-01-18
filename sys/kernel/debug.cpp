/*************************************************************************/

#include <stdio.h>

#include <kernel/arch/dconsole.h>
#include <kernel/debug.h>

/*************************************************************************/

void Debug::PrintF(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vPrintF(fmt, args);
    va_end(args);
}

/*************************************************************************/

void Debug::vPrintF(const char *fmt, va_list args)
{
    char buf[512];

    vsnprintf(buf, sizeof(buf), fmt, args);

    DebugConsole::PutString(buf, sizeof(buf));
}

/*************************************************************************/
/*
 * Display a "STOP ERROR" message and halt the system.
 */

void Debug::Panic(const char* msg)
{
    //blank_screen();

    puts("STOP ERROR: ");
    puts(msg);
    puts("\n\nTHE SYSTEM HAS BEEN HALTED");

    /* Die */
    for (;;)
        __asm __volatile("pause");
}

extern "C" void kpanic(const char *msg) { Debug::Panic(msg); }

/*************************************************************************/
