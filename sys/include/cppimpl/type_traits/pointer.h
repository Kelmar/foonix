#ifndef _X_CPPIMPL_TT_POINTER_H__
#define _X_CPPIMPL_TT_POINTER_H__

template <class T>
struct is_pointer : std::bool_constant<__is_pointer(T)>
{ };

template <class T>
constexpr bool is_pointer_v = std::is_pointer<T>::value;

// Removal of pointer
template <class T> struct remove_pointer { typedef T type; };

#define _MAKE_RM(X_) template <class T> struct remove_pointer<T X_> { typedef T X_ type; }

_MAKE_RM(*);
_MAKE_RM(* const);
_MAKE_RM(* volatile);
_MAKE_RM(* const volatile);

#undef _MAKE_RM

template <class T>
using remove_pointer_t = class remove_pointer<T>::type;

#endif /* _X_CPPIMPL_TT_POINTER_H__ */
