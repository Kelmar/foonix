/*************************************************************************/
/*************************************************************************/

#ifndef __FOONIX_KERNEL_VM_H__
#define __FOONIX_KERNEL_VM_H__

/*************************************************************************/

#include <kernel/kernel.h>
#include <kernel/kernel_args.h>

/*************************************************************************/

namespace VM
{
    void Init(KernelArgs *);

    void ReserveBootPages(addr_t physicalAddress, size_t count = 1);

    void ReservePage(addr_t start, size_t length);
}

/*************************************************************************/

#endif /* __FOONIX_KERNEL_VM_H__ */

/*************************************************************************/
