/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _X_CPPIMPL_TT_INTEGRAL_H__
#define _X_CPPIMPL_TT_INTEGRAL_H__ 1

/********************************************************************************************************************/

// Integral constant needed for almost all other checks.
template <typename T, T V>
struct integral_constant
{
    typedef T value_type;
    typedef integral_constant type;

    static constexpr T value = V;

    constexpr operator value_type() const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; }
};

/*
 * Don't think this is actually part of the C++ standard.
 * But we're adding it here none the less.
 */
template <bool V>
struct bool_constant : integral_constant<bool, V> { };

typedef bool_constant<true> true_type;
typedef bool_constant<false> false_type;

template <typename T> struct is_integral : false_type { };

// Special one offs
template <> struct is_integral<bool>        : true_type { };
template <> struct is_integral<signed char> : true_type { };

// All the below follow the same template
#define _MAKE_TRUE(X_) \
    template <> struct is_integral<X_>          : true_type { }; \
    template <> struct is_integral<unsigned X_> : true_type { }
    
_MAKE_TRUE(char);
_MAKE_TRUE(short);
_MAKE_TRUE(int);
_MAKE_TRUE(long);
_MAKE_TRUE(long long);

#undef _MAKE_TRUE

template <typename T>
constexpr bool is_integral_v = is_integral<T>::value;

/********************************************************************************************************************/

#endif /* _X_CPPIMPL_TT_INTEGRAL_H__ */

/********************************************************************************************************************/
