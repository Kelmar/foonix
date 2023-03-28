/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_ATOMIC_H__
#define __FOONIX_KERNEL_ATOMIC_H__

/********************************************************************************************************************/
/**
 * Performs an atomic compare and exchange of a 32-bit value.
 *
 * uint32_t compare_exchange(uint32_t* dest, uint32_t oldval, uint32_t newval);
 */
static inline bool atomic_cmpset_uint32(volatile uint32_t* dst, uint32_t expect, uint32_t newval)
{
    uint32_t res;

    __asm __volatile(
        "movl %2, %%eax;        "
        "lock; cmpxchg %3, %0;  "
        "pushf; popl %%eax;     "
        "andl $0x40, %%eax;     "
        "movl %%eax, %1         "
        "# atomic_cmpset_uint32 "
        : "+m"(*dst), "=r"(res)
        : "r"(expect), "r"(newval)
        : "%eax");

    return res != 0;
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_ATOMIC_H__ */

/********************************************************************************************************************/
