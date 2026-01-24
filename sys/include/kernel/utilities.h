/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_UTILITIES_H__
#define __FOONIX_KERNEL_UTILITIES_H__

/********************************************************************************************************************/

#include <type_traits>
#include <concepts>

/********************************************************************************************************************/

namespace util
{
    /// @brief Counts the number of bits set in an integer.
    inline constexpr uint
    CountBits(uintmax_t v) { return (v == 0) ? 0 : ((v & 1) + CountBits(v >> 1)); }

    /// @brief Gets the number of bits in an integer.
    template <uintmax_t V>
    struct TBitCount : std::integral_constant<int, CountBits(V)> { };

    /// @brief Gets the number of bits of an integer.
    template <uintmax_t V>
    constexpr int BitCount = TBitCount<V>::value;

    /// @brief Checks if the supplied value is a power of 2.
    template <uintmax_t V> 
    struct TIsPow2 : std::integral_constant<bool, (CountBits(V) == 1)> { };

    /// @brief Checks if the supplied value is a power of 2.
    template <uintmax_t V>
    constexpr bool IsPow2 = TIsPow2<V>::value;

    // Add alignment utilities.
    #include <kernel/utils/align.h>
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_UTILITIES_H__ */

/********************************************************************************************************************/
