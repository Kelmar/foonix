/********************************************************************************************************************/
/*
 * Driver for legacy 8259A Programmable Interrupt Controller (PIC)
 */
/********************************************************************************************************************/

#include <kernel/debug.h>

#include "cpu.h"
#include "bus.h"

#include "apic.h"

/********************************************************************************************************************/

#define PRIMARY_PIC_PORT   0x0020
#define SECONDARY_PIC_PORT 0x00A0

/********************************************************************************************************************/

#define REG_CTRL 0
#define REG_MASK 1

/********************************************************************************************************************/
/*
 * Much of this information came from the 8259A/8259A-2 data sheet published by Intel Dec 1988.
 */

/// @brief Indicator we need to write to ICW4
#define ICW1_ICW4_NEEDED     0x01

/// @brief PIC is chained to another PIC.
#define ICW1_MODE_CASCADE    0x00

/// @brief Flag indicating PIC is not chained (standalone)
#define ICW1_MODE_SINGLE     0x02

#define ICW1_INTERVAL_4      0x04
#define ICW1_INTERVAL_8      0x00

#define ICW1_LEVEL_TRIGGERED 0x08
#define ICW1_EDGE_TRIGGERED  0x00

/// @brief Indicates that this is an ICW1 request, not a OCW request.
#define ICW1_ONE             0x10

/// @brief Mask for address bits 5-7 (MCS80/85 mode only)
#define ICW1_ADDR_MASK       0xE0

/// @brief Mask for IRQ vector (80x86 mode)
#define ICW2_IRQ_MASK        0xF8

/// @brief Mask for address bits (MCS80/85 mode)
#define ICW2_ADDR_MASK       0xFE

// ICW3 bits inform PIC of a chained controller.
// In other words, that bit is the line the changed controller signals on.

/// @brief PIC in MCS80/85 mode
#define ICW4_MODE_MCS8085   0x00

/// @brief PIC in x86/x88 mode.
#define ICW4_MODE_86        0x01

/// @brief Enable auto end of interrupt mode.
#define ICW4_AUTO_EOI       0x02

// Buffering seems to be related to perhaps something like a 74LS245 in this case.

/// @brief Disable buffering
#define ICW4_NO_BUFFER      0x00

/// @brief Enable buffering as secondary PIC
#define ICW4_SECONDARY_BUF  0x08

/// @brief Enable buffering as primary PIC
#define ICW4_PRIMARY_BUF    0x0C

#define ICW4_FULLY_NESTED   0x10

/********************************************************************************************************************/
/********************************************************************************************************************/

LegacyPIC::LegacyPIC() noexcept
    : m_primary(PRIMARY_PIC_PORT, 2)
    , m_secondary(SECONDARY_PIC_PORT, 2)
{
}

/********************************************************************************************************************/
/**
 * @brief Initialize a single 8259 PIC
 * @param isPrimary Indicates if we are initializing the primary or secondary PIC
 */
void LegacyPIC::SendInit(ISABus &device, bool isPrimary)
{
    // Begin initialization of PIC, inform it we need access to command word #4
    device.byte(REG_CTRL, ICW1_ICW4_NEEDED | ICW1_ONE);
    
    // Write ICW2.  Which are the IRQ vectors
    device.byte(REG_MASK, isPrimary ? 0x20 : 0x28);

    // Map IRQ lines for chained PIC controllers
    device.byte(REG_MASK, isPrimary ? 0x04 : 0x02);

    // Inform PIC we're running on an x86 based machine.
    device.byte(REG_MASK, ICW4_MODE_86);

    // Operational Command Word 1 (OCW1)
    // Clear all interrupt masks (I.e. Allow all IRQs)
    //device.byte(REG_MASK, 0);

    // We start out with all interrupts disabled.
    device.byte(REG_MASK, 0xFF);
}

/********************************************************************************************************************/
/**
 * @brief Initialize the PIC device.
 */
void LegacyPIC::Init()
{
    SendInit(m_primary, true);
    SendInit(m_secondary, false);
}

/********************************************************************************************************************/
/**
 * @brief Send the End Of Interrupt signal to the legacy PIC devices.
 */
void LegacyPIC::SendEOI(int irqNo)
{
    // Sending the end of interrupt notices to the PIC controllers seems to be bogging the system way down.

    /*
    if (irqNo == 1)
    {
        // TODO: This needs to be moved to the ps2kbd driver.
        // Keyboard IRQ
        uint8_t scan = cpu::inb((uint16_t)0x60);
        (void)(scan);
    }
    */

    // If IRQ 8-15, we need to send an EOI to the slave controller.
    if (irqNo >= 8)
        m_secondary.byte(REG_CTRL, 0x20); // TODO: Lookup what this magic number is.

    // In all cases we need to send an EOI to the master controller.
    m_primary.byte(REG_CTRL, 0x20);
}

/********************************************************************************************************************/

void LegacyPIC::Enable(int irqNo)
{
    int bit = ~ToMaskBit(irqNo);

    // TODO: We may want to track primary/secondary enable bits so that the chained IRQs automatically get set/cleared as well.

    if (irqNo < 8)
        m_primary.byte(REG_MASK, bit);
    else
        m_secondary.byte(REG_MASK, bit);
}

/********************************************************************************************************************/

void LegacyPIC::Disable(int irqNo)
{
    int bit = ToMaskBit(irqNo);

    if (irqNo < 8)
        m_primary.byte(REG_MASK, bit);
    else
        m_secondary.byte(REG_MASK, bit);
}

/********************************************************************************************************************/
/**
 * @brief Disable all IRQs on the device.
 */
void LegacyPIC::DisableAll()
{
    m_primary.byte(REG_MASK, 0xFF);
    m_secondary.byte(REG_MASK, 0xFF);
}

/********************************************************************************************************************/
