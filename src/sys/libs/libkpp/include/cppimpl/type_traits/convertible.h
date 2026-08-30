/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _LIBKPP_CPPIMPL_TT_CONVERTIBLE_H__
#define _LIBKPP_CPPIMPL_TT_CONVERTIBLE_H__ 1

/********************************************************************************************************************/

namespace impl__
{
    // We need a version fo declval, but utilities depends on type_traits
    template <typename T>
    typename std::type_identity_t<T> declval_int() noexcept
    {
        static_assert(false, "Do not call into declval()");
    }

    template <class T>
    auto return_test(int) -> decltype(
        void (static_cast<T(*)()>(nullptr)),
        true_type { }
    );

    template <class>
    auto return_test(...) -> false_type;

    template <class From, class To>
    auto return_implicit_test(int) -> decltype(
        void (declval_int<void(&)(To)>()(declval_int<From>())),
        std::true_type { }
    );

    template <class, class>
    auto return_implicit_test(...) -> false_type;
}

template <class From, class To>
struct is_convertible : bool_constant<
    (
        decltype(impl__::return_test<To>(0))::value &&
        decltype(impl__::return_test<From, To>(0))::value
    ) ||
    (is_void<From>::value && is_void<To>::value)
> { };

template <class From, class To>
constexpr bool is_convertible_v = is_convertible<From, To>::value;

/********************************************************************************************************************/

#endif /* _LIBKPP_CPPIMPL_TT_CONVERTIBLE_H__ */

/********************************************************************************************************************/
