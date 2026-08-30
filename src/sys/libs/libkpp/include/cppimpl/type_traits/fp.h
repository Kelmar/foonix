/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _LIBKPP_CPPIMPL_TT_FP_H__
#define _LIBKPP_CPPIMPL_TT_FP_H__ 1

/********************************************************************************************************************/

template <typename T> struct is_floating_point : false_type { };

template <> struct is_floating_point<float>       : true_type { };
template <> struct is_floating_point<double>      : true_type { };
template <> struct is_floating_point<long double> : true_type { };

template <typename T>
constexpr bool is_floating_point_v = is_floating_point<T>::value;

/********************************************************************************************************************/

#endif /* _LIBKPP_CPPIMPL_TT_FP_H__ */

/********************************************************************************************************************/
