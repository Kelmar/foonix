
/********************************************************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include <kernel/arch.h>
#include <kernel/console.h>

#include "cpu.h"

/********************************************************************************************************************/

#define MAX_STACK_TRACE_DEPTH 8

/********************************************************************************************************************/

void dump_regs(const regs_t &r)
{
    console << "CPU DUMP:\r\n";

    console << "  "
        <<  "EAX: " << hex(r.eax, -8)
        << " EBX: " << hex(r.ebx, -8)
        << " ECX: " << hex(r.ecx, -8)
        << " EDX: " << hex(r.edx, -8)
        << "\r\n";

    console << "  "
        <<  "EDI: " << hex(r.edi, -8)
        << " ESI: " << hex(r.esp, -8)
        << " EBP: " << hex(r.ebp, -8)
        << " FLG: " << hex(r.eflags, -8)
        << "\r\n";

    console << "  "
        <<  " CS: " << hex(r.cs, -8)
        << " EIP: " << hex(r.eip, -8)
        << "  SS: " << hex(r.ss, -8)
        << " ESP: " << hex(r.esp, -8)
        << "\r\n";

    console << "  "
        << " DS: " << hex(r.ds, -8)
        << "  ES: " << hex(r.es, -8)
        << "  GS: " << hex(r.gs, -8)
        << "  FS: " << hex(r.fs, -8)
        << "\r\n";
   
    //Debug::PrintF("CR0: %p  CR2: %p  CR3: %p  CR4: %p\n", read_cr0(), read_cr2(), read_cr3(), read_cr4());
}

/********************************************************************************************************************/

#if 0

/*
 * TODO: Need to look into this to make sure it's valid
 *
 * The addresses I was getting back don't seem right.
 */

void cpu::stack_trace(uintptr_t stack)
{
    (void)(stack);
    uint32_t *ebp = (uint32_t *)read_ebp();
    uint32_t eip; //, *args;

    console << "Stack Trace:\r\n";

    for (int i = 0; i < MAX_STACK_TRACE_DEPTH; ++i)
    {
        eip = ebp[1];

        if (eip == 0)
        {
            console << "END OF STACK TRACE: EIP == 0\r\n";
            break;
        }

        ebp = (uint32_t *)ebp[0];
        //args = (uint32_t *)ebp[2];
        console << "  " << hex(eip, -8) << "\r\n";

        /*
        if (ebp >= start_ebp)
        {
            // Past kmain()
            console << "END OF STACK TRACE\r\n";
            break;
        }
        */
    }
}

#endif

/********************************************************************************************************************/
