/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ARCH_X86_VM_H__
#define __FOONIX_KERNEL_ARCH_X86_VM_H__

/********************************************************************************************************************/

namespace Arch::VM
{
    static const paddr_t REAL_MEMORY_END = 1024 * 1024;

    void ReserveRealMemory(paddr_t addr, size_t length);

    /// @brief Allocates a page of real memory (< 1MB)
    /// @return Returns a page alligned real memory address
    paddr_t AllocRealMemory(void);

    /// @brief Releases a page of real memory (< 1MB)
    /// @param addr An address to release.
    void ReleaseRealMemory(paddr_t addr);
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ARCH_X86_VM_H__ */

/********************************************************************************************************************/
