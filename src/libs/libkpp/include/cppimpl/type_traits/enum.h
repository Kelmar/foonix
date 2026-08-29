/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _X_CPPIMPL_TT_ENUM_H__
#define _X_CPPIMPL_TT_ENUM_H__ 1

/********************************************************************************************************************/

#define _COMP_IS_ENUM(T__) __is_enum(T__)
#define _COMP_UNDER_TYPE(T__) __underlying_type(T__)

template <typename T>
struct is_enum : bool_constant<_COMP_IS_ENUM(T)> { };

template <typename T>
constexpr bool is_enum_v = is_enum<T>::value;

template <typename T>
struct underlying_type
{
    using type = _COMP_UNDER_TYPE(T);
};

template <typename T>
using underlying_type_t = typename underlying_type<T>::type;

#undef _COMP_IS_ENUM
#undef _COMP_UNDER_TYPE

/********************************************************************************************************************/

#endif /* _X_CPPIMPL_TT_ENUM_H__ */

/********************************************************************************************************************/

