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

/// @brief Logical set of any bits required.
template <BitmaskEnum TEnum>
struct any_set : bitmask_set_t<TEnum>
{
    TEnum values;

    constexpr any_set(TEnum v) : values(v) { }

    bool operator()(TEnum other) const override
    {
        auto v = std::to_underlying(values);
        auto o = std::to_underlying(other);

        return (o & v) != 0;
    }
};

/// @brief Logical set of all bits required.
template <BitmaskEnum TEnum>
struct all_set : bitmask_set_t<TEnum>
{
    TEnum values;

    constexpr all_set(TEnum v) : values(v) { }

    bool operator()(TEnum other) const override
    {
        auto v = std::to_underlying(values);
        auto o = std::to_underlying(other);

        return (o & v) == v;
    }
};

/// @brief Alias for all_set
template <BitmaskEnum TEnum>
struct is_set : all_set<TEnum> { is_set(TEnum v) : all_set<TEnum>(v) { } };

template <BitmaskEnum TEnum>
bool operator &&(TEnum lhs, const bitmask_set_t<TEnum> &rhs)
{
    return rhs(lhs);
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
 *     if (f && is_set(MyFlags::Flag1))
 *     {
 *         // Do a thing if Flag1 set
 *     }
 *
 *     if (f && any_set(MyFlags::Flag1 | MyFlags::Flag3))
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
