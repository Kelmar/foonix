/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _X_CPPIMPL_TT_REF_H__
#define _X_CPPIMPL_TT_REF_H__ 1

/********************************************************************************************************************/

template <class T> struct remove_reference            { typedef T type; };
template <class T> struct remove_reference<T&>        { typedef T type; };
template <class T> struct remove_reference<T&&>       { typedef T type; };

template <class T>
using remove_reference_t = typename remove_reference<T>::type;

template <class T> struct is_lvalue_reference     : false_type { };
template <class T> struct is_lvalue_reference<T&> : true_type { };

template <class T> struct is_rvalue_reference      : false_type { };
template <class T> struct is_rvalue_reference<T&&> : true_type { };

template <class T>
constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

template <class T>
constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

/********************************************************************************************************************/

#endif /* _X_CPPIMPL_TT_REF_H__ */

/********************************************************************************************************************/
