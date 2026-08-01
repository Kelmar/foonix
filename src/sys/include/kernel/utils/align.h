/********************************************************************************************************************/
/*
 * Utility functions for performing alignment and alignment checks.
 */
/********************************************************************************************************************/

#ifndef FOONIX_KERNEL_UTILS_ALIGN_H__
#define FOONIX_KERNEL_UTILS_ALIGN_H__

/********************************************************************************************************************/

/// @brief Concept that restricts to values that can be aligned.
template <unsigned int V>
concept Alignable = IsPow2<V> && (V > 1);

template <uint SZ>
requires Alignable<SZ>
struct TAlignFloor 
{
    /// @brief Aligns the pointer to the previous alignment boundary.
    /// @remarks If the pointer is already aligned it will be unmodified.
    /// @param ptr The value to round.
    /// @return The value of the previous boundary, or the same boundary if the value was already aligned.
    inline constexpr uintptr_t operator ()(uintptr_t ptr) const noexcept
    {
        return ptr & ~(SZ - 1);
    }

    /// @brief Aligns the pointer to the previous alignment boundary.
    /// @remarks If the pointer is already aligned it will be unmodified.
    /// @param ptr The value to round.
    /// @return The value of the previous boundary, or the same boundary if the value was already aligned.
    inline constexpr void *operator ()(void *ptr) const noexcept
    {
        return reinterpret_cast<void *>(operator ()(reinterpret_cast<uintptr_t>(ptr)));
    }
};

/// @brief Aligns the pointer to the previous alignment boundary.
/// @remarks If the pointer is already aligned it will be unmodified.
/// @param ptr The value to round.
/// @return The value of the previous boundary, or the same boundary if the value was already aligned.
template <uint SZ>
requires Alignable<SZ>
const TAlignFloor<SZ> AlignFloor;

template <uint SZ>
requires Alignable<SZ>
struct TAlignCeiling
{
    /// @brief Aligns the pointer to the next alignment boundary.
    /// @remarks If the pointer is already aligned it will be unmodified.
    /// @param ptr The value to round.
    /// @return The value of the next boundary, or the same boundary if the value was already aligned.
    inline constexpr uintptr_t operator ()(uintptr_t ptr) const noexcept
    {
        return AlignFloor<SZ>(ptr + (SZ - 1));
    }

    /// @brief Aligns the pointer to the next alignment boundary.
    /// @remarks If the pointer is already aligned it will be unmodified.
    /// @param ptr The value to round.
    /// @return The value of the next boundary, or the same boundary if the value was already aligned.
    inline constexpr void *operator ()(void *ptr) const noexcept
    {
        return reinterpret_cast<void *>(operator ()(reinterpret_cast<uintptr_t>(ptr)));
    }
};

/// @brief Aligns the pointer to the next alignment boundary.
/// @remarks If the pointer is already aligned it will be unmodified.
/// @param ptr The value to round.
/// @return The value of the next boundary, or the same boundary if the value was already aligned.
template <uint SZ>
requires Alignable<SZ>
const TAlignCeiling<SZ> AlignCeiling;

template <uint SZ>
requires Alignable<SZ>
struct TAlignNext
{
    /// @brief Get the next alignment boundary.
    /// @remarks Unlike @ref AlignCeiling this will aways return the next alignment boundary.
    /// @param ptr The value to round.
    /// @return The value of the next boundary.
    inline constexpr uintptr_t operator ()(uintptr_t ptr) const noexcept
    {
        return AlignFloor<SZ>(ptr + SZ);
    }

    /// @brief Get the next alignment boundary.
    /// @remarks Unlike @ref AlignCeiling this will aways return the next alignment boundary.
    /// @param ptr The value to round.
    /// @return The value of the next boundary.
    inline constexpr void *operator ()(void *ptr) const noexcept
    {
        return reinterpret_cast<void *>(operator ()(reinterpret_cast<uintptr_t>(ptr)));
    }
};

/// @brief Get the next alignment boundary.
/// @remarks Unlike @ref AlignCeiling this will aways return the next alignment boundary.
/// @param ptr The value to round.
/// @return The value of the next boundary.
template <uint SZ>
requires Alignable<SZ>
const TAlignNext<SZ> AlignNext;

template <uint SZ>
requires Alignable<SZ>
struct TAlignPrev
{
    /// @brief Get the previous alignment boundary.
    /// @remarks Unlike @ref AlignFloor this will always return the previous alignment boundary.
    /// @param ptr The value to round.
    /// @return The value of the previous boundary.
    inline constexpr uintptr_t operator ()(uintptr_t ptr) const noexcept
    {
        return AlignFloor<SZ>(ptr - SZ);
    }

    /// @brief Get the previous alignment boundary.
    /// @remarks Unlike @ref AlignFloor this will always return the previous alignment boundary.
    /// @param ptr The value to round.
    /// @return The value of the previous boundary.
    inline constexpr void *operator ()(void *ptr) const noexcept
    {
        return reinterpret_cast<void *>(operator ()(reinterpret_cast<uintptr_t>(ptr)));
    }
};

/// @brief Get the previous alignment boundary.
/// @remarks Unlike @ref AlignFloor this will always return the previous alignment boundary.
/// @param ptr The value to round.
/// @return The value of the previous boundary.
template <uint SZ>
requires Alignable<SZ>
const TAlignPrev<SZ> AlignPrev;

/// @Brief checks to see if the supplied pointer is aligned.
template <uint SZ>
requires Alignable<SZ>
struct TIsAligned
{
    /// @Brief checks to see if the supplied pointer is aligned.
    inline constexpr bool operator ()(uintptr_t ptr) const noexcept
    {
        return ptr == AlignFloor<SZ>(ptr);
    }

    /// @Brief checks to see if the supplied pointer is aligned.
    inline constexpr bool operator ()(void *ptr) const noexcept
    {
        return operator ()(reinterpret_cast<uintptr_t>(ptr));
    }
};

/// @Brief checks to see if the supplied pointer is aligned.
template <uint SZ>
requires Alignable<SZ>
const TIsAligned<SZ> IsAligned;

/********************************************************************************************************************/

#endif /* FOONIX_KERNEL_UTILS_ALIGN_H__ */

/********************************************************************************************************************/
