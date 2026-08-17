/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_UTILITIES_H__
#define __FOONIX_KERNEL_UTILITIES_H__

/********************************************************************************************************************/

#include <kernel/kernel.h>
#include <kernel/types.h>

#include <type_traits>
#include <concepts>
#include <expected>
#include <string_view>

#include <kernel/span.h>

/********************************************************************************************************************/

namespace util
{
    /// @brief Counts the number of bits set in an integer.
    inline constexpr
    uint CountBits(uintmax_t v) { return (v == 0) ? 0 : ((v & 1) + CountBits(v >> 1)); }

    /// @brief Get the most significant bit set in a value.
    inline constexpr
    uint MSB(uintmax_t v)
    {
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;

#if UINT_MAX == UINT64_MAX
        v |= v >> 32;
#endif

        return v & ~(v >> 1);
    }

    /**
     * @brief Get the next power of 2 above the supplied value.
     *
     * This function will return the next power of two that will contain the supplied value.
     *
     * For example:
     * int i = NextPow2(640);  // Returns 1024
     * int n = NextPow2(1024); // Returns 2048
     */
    inline constexpr
    uintmax_t NextPow2(uintmax_t v) { return (v == 0) ? 1 : (MSB(v) << 1); }

    /**
     * @brief Parse a string_view for an integer value.
     *
     * Supports handling of 0x and $ prefixes for hexidecimal numbers.
     *
     * Returns a Kernel::ErrorCode if an integer cannot be parsed. Otherwise the value parsed is returned.
     */ 
    std::expected<int, Kernel::ErrorCode> parseInt(const std::string_view &);

    /************************************************************************************************************/
    // STL like evaluations of the some of the above functions.

    /// @brief Gets the number of bits in an integer.
    template <uintmax_t V>
    struct bit_count : std::integral_constant<int, CountBits(V)> { };

    /// @brief Gets the number of bits of an integer.
    template <uintmax_t V>
    constexpr int bit_count_v = bit_count<V>::value;

    /// @brief Checks if the supplied value is a power of 2.
    template <uintmax_t V> 
    struct is_pow2 : std::integral_constant<bool, (CountBits(V) == 1)> { };

    /// @brief Checks if the supplied value is a power of 2.
    template <uintmax_t V>
    constexpr bool is_pow2_v = is_pow2<V>::value;

    template <uintmax_t V>
    struct msb : std::integral_constant<int, MSB(V)> { };

    template <uintmax_t V>
    constexpr uintmax_t msb_v = msb<V>::value;

    template <uintmax_t V>
    struct next_pow2 : std::integral_constant<int, NextPow2(V)> { };

    template <uintmax_t V>
    constexpr uintmax_t next_pow2_v = next_pow2<V>::value;

    /************************************************************************************************************/

    // Add alignment utilities.
    #include <kernel/utils/align.h>
}

#include <kernel/utils/flags.h>

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_UTILITIES_H__ */

/********************************************************************************************************************/
