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

// TODO: See if there's a way we can do this with concepts and no virtual method call.

template <BitmaskEnum TEnum>
struct bitmask_set_t
{
    virtual bool operator()(TEnum other) const = 0;
};

template <BitmaskEnum TEnum>
constexpr bool has_any(TEnum v, TEnum items)
{
    using basetype = std::underlying_type_t<TEnum>;
    return (static_cast<basetype>(v) & static_cast<basetype>(items)) != 0;
}

template <BitmaskEnum TEnum>
constexpr bool has_all(TEnum v, TEnum items)
{
    using basetype = std::underlying_type_t<TEnum>;
    return (static_cast<basetype>(v) & static_cast<basetype>(items)) == static_cast<basetype>(items);
}

/// @brief Alias for all_set
template <BitmaskEnum TEnum>
constexpr bool is_set(TEnum v, TEnum items)
{
    return has_all(v, items);
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
 *     if (is_set(f, MyFlags::Flag1))
 *     {
 *         // Do a thing if Flag1 set
 *     }
 *
 *     if (any_set(f, MyFlags::Flag1 | MyFlags::Flag3))
 *     {
 *         // Do a thing if Flag1 or Flag3 are set.
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
