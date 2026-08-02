/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _X_CPPIMPL_TT_VOID_H__
#define _X_CPPIMPL_TT_VOID_H__ 1

/********************************************************************************************************************/

template <typename T> struct is_void : false_type { };

template <> struct is_void<void>                : true_type { };
template <> struct is_void<void const>          : true_type { };
template <> struct is_void<void volatile>       : true_type { };
template <> struct is_void<void const volatile> : true_type { };

template <typename T>
constexpr bool is_void_v = is_void<T>::value;

/********************************************************************************************************************/

#endif /* _X_CPPIMPL_TT_VOID_H__ */

/********************************************************************************************************************/
