/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_X86_APIC_H__
#define __FOONIX_ARCH_X86_APIC_H__

/********************************************************************************************************************/

#include "kernel/utils/nocopy.h"

#include "cpu.h"
#include "bus.h"

/********************************************************************************************************************/

// TODO: Should probably be moved into the timer driver.
#define TIMER_IRQ 0

/********************************************************************************************************************/

/**
 * @brief Driver for legacy 8259A Programmable Interrupt Controller (PIC)
 */
class LegacyPIC : util::nocopy, util::nomove
{
private:
    ISABus m_primary;
    ISABus m_secondary;

    void SendInit(ISABus &device, bool isPrimary);

    int ToMaskBit(int irqNo) { return 1 << (irqNo & 0x08); }

public:
    LegacyPIC() noexcept;
    ~LegacyPIC() { }

    /**
     * @brief Initialize the PIC device.
     */
    void Init();

    /**
     * @brief Send the End Of Interrupt signal to the legacy PIC devices.
     */
    void SendEOI(int irqNo);

    /// @brief Enable a given IRQ.
    void Enable(int irqNo);

    /// @brief Disable an IRQ.
    void Disable(int irqNo);

    /**
     * @brief Disable all IRQs on the device.
     */
    void DisableAll();
};

/********************************************************************************************************************/

namespace cpu
{
    extern LegacyPIC s_legacyPIC;

    void InitPIC();
}

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_X86_APIC_H__ */

/********************************************************************************************************************/
