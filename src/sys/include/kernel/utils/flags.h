/********************************************************************************************************************/
/*
 * Utility functions for performing operations on bit flags.
 */
/********************************************************************************************************************/

#ifndef FOONIX_KERNEL_UTILS_FLAGS_H__
#define FOONIX_KERNEL_UTILS_FLAGS_H__

/********************************************************************************************************************/

#include <type_traits>
#include <utility>

template <typename TEnum>
struct enable_flag_ops : std::false_type { };

template <typename TEnum>
concept BitmaskEnum = enable_flag_ops<TEnum>::value;

#define DECLARE_BIT_OP(OP_) \
    template <BitmaskEnum TEnum> \
    constexpr TEnum operator OP_(TEnum lhs, TEnum rhs) { \
       return static_cast<TEnum>(std::to_underlying(lhs) OP_ std::to_underlying(rhs)); } \
    template <BitmaskEnum TEnum> \
    constexpr TEnum &operator OP_ ## =(TEnum &lhs, TEnum rhs) { lhs = lhs OP_ rhs; return lhs; }

DECLARE_BIT_OP(|)
DECLARE_BIT_OP(&)
DECLARE_BIT_OP(^)

#undef DECLARE_BIT_OP

template <BitmaskEnum TEnum>
constexpr TEnum operator ~(TEnum flag)
{
    return static_cast<TEnum>(~std::to_underlying(flag));
}

template <BitmaskEnum TEnum>
constexpr TEnum operator +=(TEnum &lhs, TEnum rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template <BitmaskEnum TEnum>
constexpr TEnum operator -=(TEnum &lhs, TEnum rhs)
{
    lhs = lhs & (~rhs);
    return lhs;
}

template <BitmaskEnum TEnum>
constexpr bool has_flags(TEnum value, TEnum flags)
{
    return (value & flags) == flags;
}

template <BitmaskEnum TEnum>
constexpr bool has_any(TEnum value, TEnum flags)
{
    using T = std::underlying_type_t<TEnum>;
    return static_cast<T>(value & flags) != 0;
}

/*
 * Allows us to define a enum class with bits and perform ops on them:
 *
 * enum class MyFlags : uint32_t { Flag1 = 0x01, Flag2 = 0x02, Flag3 = 0x04 };
 *
 * as_flags(MyFlags);
 *
 *
 * void doFlagThing()
 * {
 *     MyFlags f = MyFlags::Flag1 | MyFlags::Flag2;
 *
 *     if (has_flags(f, MyFlags::Flag1))
 *     {
 *         // do a thing if Flag1 set
 *     }
 *
 *     f += MyFlags::Flag3; // Add a flag
 *     f -= MyFlags::Flag2; // Remove a flag
 *
 *     f |= MyFlags::Flag3; // Alt to add a flag
 *     f &= ~MyFlags::Flag1; // Alt to remove a flag
 *
 *     f ^= MyFlags::Flag1; // Toggle a flag
 * }
 */
#define as_flags(X__) template<> struct enable_flag_ops<X__> : std::true_type { }

/********************************************************************************************************************/

#endif /* FOONIX_KERNEL_UTILS_FLAGS_H__ */

/********************************************************************************************************************/
