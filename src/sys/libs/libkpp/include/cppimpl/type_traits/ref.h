/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _LIBKPP_CPPIMPL_TT_REF_H__
#define _LIBKPP_CPPIMPL_TT_REF_H__ 1

/********************************************************************************************************************/

namespace impl__
{
    template <class T>
    auto try_add_lvalue(int) -> type_identity<T&>;

    template <class T>
    auto try_add_lvalue(...) -> type_identity<T>;

    template <class T>
    auto try_add_rvalue(int) -> type_identity<T&&>;

    template <class T>
    auto try_add_rvalue(...) -> type_identity<T>;
}

template <class T> struct remove_reference            { typedef T type; };
template <class T> struct remove_reference<T&>        { typedef T type; };
template <class T> struct remove_reference<T&&>       { typedef T type; };

template <class T>
using remove_reference_t = typename remove_reference<T>::type;

template <class T> struct is_lvalue_reference     : false_type { };
template <class T> struct is_lvalue_reference<T&> : true_type { };

template <class T> struct is_rvalue_reference      : false_type { };
template <class T> struct is_rvalue_reference<T&&> : true_type { };

template <class T> struct add_lvalue_reference : decltype(impl__::try_add_lvalue<T>(0)) { };
template <class T> struct add_rvalue_reference : decltype(impl__::try_add_rvalue<T>(0)) { };

template <class T>
constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

template <class T>
constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

template <class T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

template <class T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

/********************************************************************************************************************/

#endif /* _LIBKPP_CPPIMPL_TT_REF_H__ */

/********************************************************************************************************************/
