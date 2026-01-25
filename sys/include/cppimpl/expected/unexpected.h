/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _X_CPPIMPL_EX_UNEXPECTED_H__
#define _X_CPPIMPL_EX_UNEXPECTED_H__ 1

/********************************************************************************************************************/

// Not a fully standards compliant version, but should suffice for our needs.

template <class Err>
class unexpected
{
private:
    Err m_error;

public:
    constexpr unexpected(const unexpected &) = default;
    constexpr unexpected(unexpected &&) = default;

    constexpr unexpected(Err &&e)
        : m_error(std::forward(e))
    {
    }

    constexpr const Err &error() const & noexcept { return m_error; }
    constexpr Err &error() & noexcept { return m_error; }

    constexpr const Err &&error() const && noexcept { return m_error; }
    constexpr Err &&error() && noexcept { return m_error; }

    constexpr unexpected &operator =(const unexpected &) = default;
    constexpr unexpected &operator =(unexpected &&) = default;

    template <class E2>
    friend constexpr bool operator ==(const unexpected &lhs, const unexpected<E2> &rhs)
    {
        return lhs.m_error == rhs.m_error;
    }
};

/********************************************************************************************************************/

#endif /* _X_CPPIMPL_EX_UNEXPECTED_H__ */

/********************************************************************************************************************/
