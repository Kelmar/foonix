/********************************************************************************************************************/
/*
 * Boot driver for 8259A Programmable Interrupt Controller (PIC)
 */
/********************************************************************************************************************/

#include <kernel/debug.h>

#include "cpu.h"
#include "bus.h"

/********************************************************************************************************************/

#define PRIMARY_PIC_PORT   0x0020
#define SECONDARY_PIC_PORT 0x00A0

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

static
void init_pic_device(int port, bool isPrimary)
{
    bus pic((void*)port, 2);

    // Begin initialization of PIC, inform it we need access to command word #4
    pic.byte(0, ICW1_ICW4_NEEDED | ICW1_ONE);
    
    // Write ICW2.  Which are the IRQ vectors
    pic.byte(1, isPrimary ? 0x20 : 0x28);

    // Map IRQ lines for chained PIC controllers
    pic.byte(1, isPrimary ? 0x04 : 0x02);

    // Inform PIC we're running on an x86 based machine.
    pic.byte(1, ICW4_MODE_86);

    // Operational Command Word 1 (OCW1)
    // Clear all interrupt masks (I.e. Allow all IRQs)
    //pic.byte(1, 0);

    pic.byte(1, isPrimary ? 0xFE : 0xFF); // Disable all but timer interrupt.
}

/********************************************************************************************************************/

void init_pics()
{
    init_pic_device(PRIMARY_PIC_PORT, true);
    init_pic_device(SECONDARY_PIC_PORT, false);
}

/********************************************************************************************************************/

void pic_send_eoi(int irq_no)
{
    // Sending the end of interrupt notices to the PIC controllers seems to be bogging the system way down.

    bus primary_pic((void*)PRIMARY_PIC_PORT, 2);

    if (irq_no == 1)
    {
        // Keyboard IRQ
        uint8_t scan = cpu::inb((uint16_t)0x60);
        (void)(scan);
    }

    // If IRQ 8-15, we need to send an EOI to the slave controller.
    if (irq_no >= 8)
    {
        bus secondary_pic((void*)SECONDARY_PIC_PORT, 2);
        secondary_pic.byte(0, 0x20);
    }

    // In all cases we need to send an EOI to the master controller.
    primary_pic.byte(0, 0x20);
}

/********************************************************************************************************************/
