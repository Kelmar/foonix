/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_INTERRUPT_H__
#define __FOONIX_KERNEL_INTERRUPT_H__

/********************************************************************************************************************/

#include "cpu.h"

/********************************************************************************************************************/

namespace cpu
{
    /**
     * @brief Interrupt enable disable stack
     *
     * @details This class manages the number of calls to PushDisable() and PopDisable(), and will automatically
     * call the cpu::stop_interrupts() or cpu::start_interrupts() when the counts are > 0 or == 0 respectively.
     */ 
    class InterruptStack
    {
    private:
        int m_disableCount;

    public:
        constexpr InterruptStack(void) noexcept
            : m_disableCount(0)
        {
        }

        ~InterruptStack() { }

        void PushDisable() noexcept
        {
            if (++m_disableCount)
                cpu::stop_interrupts();
        }

        void PopDisable() noexcept
        {
            if (--m_disableCount == 0)
                cpu::start_interrupts();
        }
    };

    extern InterruptStack interrupt_stack; // Defined in main.cpp
}

/// @brief Class to automatically call PushDisable and PopDisable on the global cpu::interrupt_stack
struct AutoInterruptDisable
{
    AutoInterruptDisable() noexcept { cpu::interrupt_stack.PushDisable(); }
    ~AutoInterruptDisable() noexcept { cpu::interrupt_stack.PopDisable(); }
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_INTERRUPT_H__ */

/********************************************************************************************************************/
