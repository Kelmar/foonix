/********************************************************************************************************************/
/*
 * Various bit math functions.
 */
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_MATH_BIT_H__
#define __FOONIX_KERNEL_MATH_BIT_H__

/********************************************************************************************************************/

/**
 * @brief Get the index of the most significant bit set.
 *
 * @return The index of the most significant bit (one based), or 0 if no bits are set.
 */
template <std::unsigned_integral T>
inline
constexpr size_t msb_index(T v)
{
    // Probably some other better ways to do this.
    return (v > 1) ? (const_msb_index(v >> 1) + 1) : v;
}

#ifdef __GNUC__
/**
 * @brief Get the index of the most significant bit set.
 *
 * @return The index of the most significant bit (one based), or 0 if no bits are set.
 */
inline
constexpr size_t msb_index(uint64_t v)
{
    return (v == 0) ? 0 : (64 - __builtin_clzll(v));
}

/**
 * @brief Get the index of the most significant bit set.
 *
 * @return The index of the most significant bit (one based), or 0 if no bits are set.
 */
inline
constexpr size_t msb_index(uint32_t v)
{
    return (v == 0) ? 0 : (32 - __builtin_clzl(v));
}
#endif

/********************************************************************************************************************/

/**
 * @brief Get the most significant bit set in a value.
 *
 * @return Returns the same value that was passed in, but with only the most significant bit left set.
 */
template <std::unsigned_integral T>
inline
constexpr T msb(T v)
{
    return (v == 0) ? 0 : static_cast<T>((1 << msb_index(v)));
}

/********************************************************************************************************************/

/**
 * @brief Get the index of the least significant bit set.
 *
 * @return The index of the least significant bit (one based), or 0 if no bits are set.
 */
template <std::unsigned_integral T>
inline
constexpr size_t lsb_index(T v)
{
    // There are faster ways to do this with fancy bit twiddles and lookups.
    if (v == 0)
        return 0;

    return (v & 1) ? 1 : (lsb_index(v >> 1) + 1);
}


#ifdef __GNUC__
/**
 * @brief Get the index of the least significant bit set.
 *
 * @return The index of the least significant bit (one based), or 0 if no bits are set.
 */
inline
constexpr size_t lsb_index(uint64_t v)
{
    return __builtin_ffsll(v);
}

/**
 * @brief Get the index of the least significant bit set.
 *
 * @return The index of the least significant bit (one based), or 0 if no bits are set.
 */
inline
constexpr size_t lsb_index(uint32_t v)
{
    return __builtin_ffsl(v);
}
#endif

/********************************************************************************************************************/

/**
 * @brief Get the least significant bit set in a value.
 *
 * @return Returns the same value that was passed in, but with only the least significant bit left set.
 */
template <std::unsigned_integral T>
inline
constexpr T lsb(T v)
{
    //return (v & static_cast<T>(-static_cast<signed T>(v)));
    return (v & static_cast<T>(-(v)));
}

/********************************************************************************************************************/

/**
 * @brief Return the number of bits set in the given unsigned integral value.
 */
template <std::unsigned_integral T>
inline
constexpr size_t popcount(T v)
{
    return (v == 0) ? 0 : ((v & 1) + popcount(v >> 1));
}

#ifdef __GNUC__
/**
 * @brief Return the number of bits set in the given unsigned integral value.
 */
inline constexpr size_t popcount(uint32_t v)
{
    return __builtin_popcountl(v);
}

/**
 * @brief Return the number of bits set in the given unsigned integral value.
 */
inline constexpr size_t popcount(uint64_t v)
{
    return __builtin_popcountll(v);
}
#endif /* __GNUC__ */

/********************************************************************************************************************/

/**
 * @brief Checks if the supplied number is a power of 2 or not.
 */
template <std::unsigned_integral T>
inline
constexpr bool is_pow2(T v)
{
    return v != 0 && (v & (v - 1)) == 0;
}

/********************************************************************************************************************/
/**
 * @brief Get the next power of 2 above the supplied value.
 *
 * This function will return the next power of two that will contain the supplied value.
 *
 * For example:
 * int i = NextPow2(640);  // Returns 1024
 * int n = NextPow2(1024); // Returns 2048
 */
template <std::unsigned_integral T>
inline
constexpr T next_pow2(T v)
{
    return (v == 0) ? 1 : (msb(v) << 1);
}

/********************************************************************************************************************/
// STL like evaluations of the some of the above functions, useful for template metaprogramming.

/// @brief Get the index of the most significant bit set.
template <size_t SZ>
struct msb_index_t : std::integral_constant<size_t, msb_index(SZ)> { };

/// @brief Get the index of the most significant bit set.
template <size_t SZ>
constexpr size_t msb_index_v = msb_index_t<SZ>::value;

/// @brief Get the most significant bit set in a value.
template <size_t SZ>
struct msb_t : std::integral_constant<size_t, msb(SZ)> { };

/// @brief Get the most significant bit set in a value.
template <size_t SZ>
constexpr size_t msb_v = msb_t<SZ>::value;

/// @brief Get the index of the least significant bit set.
template <size_t SZ>
struct lsb_index_t : std::integral_constant<size_t, lsb_index(SZ)> { };

/// @brief Get the index of the least significant bit set.
template <size_t SZ>
constexpr size_t lsb_index_v = lsb_index_t<SZ>::value;

/// @brief Get the least significant bit set in a value.
template <size_t SZ>
struct lsb_t : std::integral_constant<size_t, lsb(SZ)> { };

/// @brief Get the least significant bit set in a value.
template <size_t SZ>
constexpr size_t lsb_v = lsb_t<SZ>::value;

/// @brief Gets the number of bits of an integer.
template <size_t SZ>
struct popcount_t : std::integral_constant<int, popcount(SZ)> { };

/// @brief Gets the number of bits of an integer.
template <size_t SZ>
constexpr size_t popcount_v = popcount_t<SZ>::value;

/// @brief Checks if the supplied number is a power of 2 or not.
template <size_t SZ>
struct is_pow2_t : std::integral_constant<bool, is_pow2(SZ)> { };

/// @brief Checks if the supplied number is a power of 2 or not.
template <size_t SZ>
constexpr bool is_pow2_v = is_pow2_t<SZ>::value;

/**
 * @brief Get the next power of 2 above the supplied value.
 *
 * This function will return the next power of two that will contain the supplied value.
 *
 * For example:
 * int i = NextPow2(640);  // Returns 1024
 * int n = NextPow2(1024); // Returns 2048
 */
template <size_t SZ>
struct next_pow2_t : std::integral_constant<size_t, next_pow2(SZ)> { };

/**
 * @brief Get the next power of 2 above the supplied value.
 *
 * This function will return the next power of two that will contain the supplied value.
 *
 * For example:
 * int i = NextPow2(640);  // Returns 1024
 * int n = NextPow2(1024); // Returns 2048
 */
template <size_t SZ>
constexpr size_t next_pow2_v = next_pow2_t<SZ>::value;

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_MATH_BIT_H__ */

/********************************************************************************************************************/
