/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _LIBKPP_CPPIMPL_TT_SIGN_H__
#define _LIBKPP_CPPIMPL_TT_SIGN_H__ 1

/********************************************************************************************************************/

namespace __impl
{
    template <typename T, bool = is_arithmetic<T>::value>
    struct is_unsigned_check : integral_constant<bool, T(0) < T(-1)> { };

    template <typename T>
    struct is_unsigned_check<T, false> : false_type { };

    template <typename T, bool = is_arithmetic<T>::value>
    struct is_signed_check : integral_constant<bool, T(-1) < T(0)> { };

    template <typename T>
    struct is_signed_check<T, false> : false_type { };
}

template <typename T>
struct is_unsigned : __impl::is_unsigned_check<T>::type { };

template <typename T>
struct is_signed : __impl::is_signed_check<T>::type { };

template <typename T>
constexpr bool is_unsigned_v = is_unsigned<T>::value;

template <typename T>
constexpr bool is_signed_v = is_signed<T>::value;

/********************************************************************************************************************/

#endif /* _LIBKPP_CPPIMPL_TT_SIGN_H__ */

/********************************************************************************************************************/
