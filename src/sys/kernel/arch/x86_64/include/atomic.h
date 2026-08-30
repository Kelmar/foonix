/********************************************************************************************************************/
/*
 * Some pulled from FreeBSD i368 atomic.h
 *
 *
 * Copyright (c) 1998 Doug Rabson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_X86_64_ATOMIC_H__
#define __FOONIX_ARCH_X86_64_ATOMIC_H__

#include <stdint.h>

#include "atomic_base.h"

/********************************************************************************************************************/
/*
 * x86 64-bit specific atomic operations.
 */

namespace atomic
{
    static inline FORCE_INLINE
    uint64_t xchg(volatile uint64_t *addr, uint64_t value)
    {
        uint64_t result;

        __asm__ volatile(
            "lock; xchgq %0, %1"
            : "+m" (*addr), "=a" (result)
            : "1" (value)
            : "memory", "cc");

        return result;
    }

    // Modified version from FreeBSD
    static inline FORCE_INLINE
    bool cmpset(volatile uint64_t *addr, uint64_t expected, uint64_t value)
    {
        uint8_t result;

        __asm__ volatile(
            " lock; cmpxchg %3, %1 ; "
            : "=@cce" (result), /* 0 */
              "+m" (*addr),     /* 1 */
              "+a" (expected)   /* 2 */
            : "r" (value)       /* 3 */
            : "memory", "cc");

        return (result != 0);
    }

    // Wrapper around cmpxchg16b
    template <typename T>
        requires (sizeof(T) == 16)
    static inline FORCE_INLINE
    bool cmpset(volatile T *addr, T expected, T value)
    {
        uint8_t result;

        __asm__ volatile(
            "  lock; cmpxchg16b %1 ; "
            "  sete %0"
            : "=q" (result),                /* 0 */
              "+m" (*addr),                 /* 1 */
              "+A" (expected)               /* 2 */
            : "b" ((uint64_t)value),        /* 3 */
              "c" ((uint64_t)(value >> 64)) /* 4 */
            : "memory", "cc");

        return (result != 0);
    }
}

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_X86_64_ATOMIC_H__ */

/********************************************************************************************************************/
