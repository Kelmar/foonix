/*************************************************************************/

/*************************************************************************/

#ifndef MATH_OS_H__
#define MATH_OS_H__

/*************************************************************************/

namespace impl__
{
    inline
        constexpr size_t SubLog2(size_t v, size_t l, size_t i)
    {
        return l >= v ? i : SubLog2(v, l << 1, i + 1);
    }
}

/**
 * @brief Returns the highest bit set in a size.
 * @param v The size to check
 * @return A value with only the highest bit set.
 */
inline
constexpr size_t HighBit(size_t v)
{
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);
    v |= (v >> 16);
#if SYSTEM_BITS > 32
    v |= (v >> 32);
#endif

    return (v & ~(v >> 1));
}

/**
 * @brief Convert to the smallest power of 2 that will fit the requested size.
 * @param v The size requested
 * @return A number that is a power of two that is equal to or greater than v.
 */
inline
constexpr size_t ToPow2(size_t v)
{
    size_t hi = HighBit(v);
    return hi < v ? hi << 1 : hi;
}

/**
 * @brief Gets log base 2 of the requested value.
 * @param v Value to get the log base 2 of.
 */
inline
constexpr size_t Log2(size_t v)
{
    return impl__::SubLog2(v, 1, 0);
}

/*************************************************************************/

#endif /* MATH_OS_H__ */

/*************************************************************************/
