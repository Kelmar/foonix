/*************************************************************************/
/*************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_H__
#define __FOONIX_KERNEL_ARCH_H__

/*************************************************************************/

#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/kernel_args.h>

/*************************************************************************/

namespace Arch
{
    /**
     * @brief Initialize boot memory map
     * 
     * @remark
     * This is so we can start reserving some initial pages for the paging
     * tables and what have you.
     * 
     * @param ka Kernel arguments to be filled in with a memory map.
     */
    void InitBootMemory(KernelArgs *ka);

    /**
     * @brief Initialize the paging system on the CPU
     * 
     * @remark
     * This initializes the proper paging system with the CPU for the kernel.
     * 
     * @param ka Kernel arguments
     * 
     * @returns A kernel error code indicating the result of the operation.
     */
    Kernel::ErrorCode InitPaging(KernelArgs *);
}

/*************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_H__ */

/*************************************************************************/
