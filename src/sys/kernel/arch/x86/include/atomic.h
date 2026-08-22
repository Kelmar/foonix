/********************************************************************************************************************/
/*
 * Common instructions that do not vary from x32 and x64
 */
/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_X86_ATOMIC_H__
#define __FOONIX_ARCH_X86_ATOMIC_H__

/********************************************************************************************************************/

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

#include <type_traits>

/********************************************************************************************************************/

#define MEMORY_BARRIER() __asm__ volatile("" : : : "memory")

namespace atomic
{
    /**
     * @brief Set's the value at the given address and returns the old value atomically.
     * @return The old value that was at the address before the exchange.
     */
    static inline FORCE_INLINE
    uint8_t xchg(volatile uint8_t *addr, uint8_t value)
    {
        uint8_t rval;
        __asm__ volatile("lock; xchgb %0, %1" : "+m" (*addr), "=a" (rval) : "1" (value) : "cc");
        return rval;
    }

    /**
     * @brief Set's the value at the given address and returns the old value atomically.
     * @return The old value that was at the address before the exchange.
     */
    static inline FORCE_INLINE
    uint16_t xchg(volatile uint16_t *addr, uint16_t value)
    {
        uint16_t rval;
        __asm__ volatile("lock; xchgw %0, %1" : "+m" (*addr), "=a" (rval) : "1" (value) : "cc");
        return rval;
    }

    /**
     * @brief Set's the value at the given address and returns the old value atomically.
     * @return The old value that was at the address before the exchange.
     */
    static inline FORCE_INLINE
    uint32_t xchg(volatile uint32_t *addr, uint32_t value)
    {
        uint32_t rval;
        __asm__ volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (rval) : "1" (value) : "memory", "cc");
        return rval;
    }

    /**
     * @brief Atomically stores a value at a memory address.
     */
    template <typename T>
    inline void store(volatile T *addr, std::type_identity_t<T> value)
    {
        static_assert(std::is_integral_v<T>);
        MEMORY_BARRIER();
        *addr = value;
    }

    /**
     * @brief Atomically reads a value at a memory address.
     */
    template <typename T>
    inline std::type_identity_t<T> load(volatile const T *addr)
    {
        static_assert(std::is_integral_v<T>);
        T rval = *addr;
        MEMORY_BARRIER();
        return rval;
    }
}

#undef MEMORY_BARRIER

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_X86_ATOMIC_H__ */

/********************************************************************************************************************/
