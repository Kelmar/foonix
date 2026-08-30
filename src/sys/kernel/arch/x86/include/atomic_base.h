/********************************************************************************************************************/
/*
 * Atomic instructions that do not vary from x32 and x64
 */
/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_X86_ATOMIC_H__
#define __FOONIX_ARCH_X86_ATOMIC_H__

/********************************************************************************************************************/

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

#include <type_traits>
#include <concepts>

/********************************************************************************************************************/

#define MEMORY_BARRIER() __asm__ volatile("" : : : "memory")

namespace atomic
{
    /************************************************************************************************************/
    /*
     * Unconditional exchange.
     */

#define BUILD_xchg(T__, X__)                  \
    static inline FORCE_INLINE                \
    T__ xchg(volatile T__ *addr, T__ value) { \
        T__ result;                           \
        __asm__ volatile(                     \
            "lock; xchg" X__ " %0, %1"        \
            : "+m" (*addr), "=a" (result)     \
            : "1" (value)                     \
            : "memory", "cc");                \
        return result; }

    BUILD_xchg(uint8_t, "b")

    BUILD_xchg(uint16_t, "w")

    BUILD_xchg(uint32_t, "l")
    
#undef BUILD_xchg

    /************************************************************************************************************/
    /*
     * Compare and set
     */

#define BUILD_cmpset(T__, X__)                                  \
    static inline FORCE_INLINE                                  \
    bool cmpset(volatile T__ *addr, T__ expected, T__ value) {  \
        uint8_t result;                                         \
        __asm__ volatile(                                       \
            "lock; cmpxchg" X__ " %3, %0; "                     \
            "sete %1"                                           \
            : "+m" (addr), "=q" (result), "+a" (expected)       \
            : "q" (value)                                       \
            : "memory", "cc");                                  \
        return (result != 0); }

    BUILD_cmpset(uint8_t, "b")

    BUILD_cmpset(uint16_t, "w")

    BUILD_cmpset(uint32_t, "l")

#undef BUILD_cmpset

    /************************************************************************************************************/
    /*
     * Store with memory barrier
     */

    /**
     * @brief Atomically stores a value at a memory address.
     */
    template <std::unsigned_integral T>
    inline FORCE_INLINE
    void store(volatile T *addr, std::type_identity_t<T> value)
    {
        MEMORY_BARRIER();
        *addr = value;
    }

    /************************************************************************************************************/
    /*
     * Load with memory barrier.
     */

    /**
     * @brief Atomically reads a value at a memory address.
     */
    template <std::unsigned_integral T>
    inline FORCE_INLINE
    std::type_identity_t<T> load(volatile const T *addr)
    {
        T rval = *addr;
        MEMORY_BARRIER();
        return rval;
    }

    /************************************************************************************************************/
}

#undef MEMORY_BARRIER

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_X86_ATOMIC_H__ */

/********************************************************************************************************************/
