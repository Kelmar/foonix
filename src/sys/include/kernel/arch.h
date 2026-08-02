/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_H__
#define __FOONIX_KERNEL_ARCH_H__

/********************************************************************************************************************/

#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/kernel_args.h>

#include <kernel/arch/dconsole.h>

/********************************************************************************************************************/

namespace arch
{
    /**
     * @brief Read bootloader information (if available).
     *
     * @details
     * This function should read any information from the bootloader, firmware, or whatever that is needed to get
     * the kernel up and running.  This includes things like memory maps, command line arguments, etc.
     */
    void Init(KernelArgs *ka);
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_H__ */

/********************************************************************************************************************/
