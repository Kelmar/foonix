/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _X_CPPIMPL_EX_EXPECTED_H__
#define _X_CPPIMPL_EX_EXPECTED_H__ 1

/********************************************************************************************************************/

// Not a fully standards compliant version, but should suffice for our needs.

template <typename TRes, typename TErr>
class expected
{
public:
    typedef TRes value_type;
    typedef TErr error_type;
    typedef unexpected<error_type> unexpected_type;

private:
    bool m_hasValue;

    union wrapper__
    {
        value_type value;
        error_type error;

        constexpr wrapper__(value_type v) noexcept : value(v) {}
        constexpr wrapper__(error_type e) noexcept : error(e) {}
    } m_wrapper;

public:
    constexpr expected(value_type value) noexcept
        : m_hasValue(true)
        , m_wrapper(value)
    {
    }

    constexpr expected(error_type error) noexcept
        : m_hasValue(false)
        , m_wrapper(error)
    {
    }

    constexpr expected(unexpected_type error) noexcept
        : m_hasValue(false)
        , m_wrapper(error.error())
    {
    }

    constexpr bool has_value() const noexcept { return m_hasValue; }

    constexpr const value_type &value() const &
    {
        if (!m_hasValue)
            kpanic("Attempt to get value from unexpected result.");
        return m_wrapper.value;
    }

    constexpr value_type &value() &
    {
        if (!m_hasValue)
            kpanic("Attempt to get value from unexpected result.");
        return m_wrapper.value;
    }

    constexpr const value_type &&value() const &&
    {
        if (!m_hasValue)
            kpanic("Attempt to get value from unexpected result.");
        return std::move(m_wrapper.value);
    }

    constexpr value_type &&value() &&
    {
        if (!m_hasValue)
            kpanic("Attempt to get value from unexpected result.");
        return std::move(m_wrapper.value);
    }

    constexpr error_type &error() &
    {
        if (m_hasValue)
            kpanic("Attempt to get error from valid result.");

        return m_wrapper.error;
    }

    constexpr const error_type &error() const &
    {
        if (m_hasValue)
            kpanic("Attempt to get error from valid result.");

        return m_wrapper.error;
    }

    constexpr error_type &&error() &&
    {
        if (m_hasValue)
            kpanic("Attempt to get error from valid result.");

        return std::move(m_wrapper.error);
    }

    constexpr const error_type &&error() const &&
    {
        if (m_hasValue)
            kpanic("Attempt to get error from valid result.");

        return std::move(m_wrapper.error);
    }

    constexpr operator bool() const noexcept { return has_value(); }
};

template <typename TErr>
class expected<void, TErr>
{
public:
    typedef TErr error_type;
    typedef unexpected<error_type> unexpected_type;

private:
    bool m_hasValue;
    error_type m_error;

public:
    constexpr expected() 
        : m_hasValue(true)
        , m_error()
    {
    }

    constexpr expected(error_type error)
        : m_hasValue(false)
        , m_error()
    {
    }

    constexpr expected(unexpected_type error)
        : m_hasValue(false)
        , m_error(error.error())
    {
    }

    constexpr bool has_value() const noexcept { return m_hasValue; }
    
    constexpr error_type &error() &
    {
        if (m_hasValue)
            kpanic("Attempt to get error from valid result.");

        return m_error;
    }

    constexpr const error_type &error() const &
    {
        if (m_hasValue)
            kpanic("Attempt to get error from valid result.");

        return m_error;
    }

    constexpr error_type &&error() &&
    {
        if (m_hasValue)
            kpanic("Attempt to get error from valid result.");

        return std::move(m_error);
    }

    constexpr const error_type &&error() const &&
    {
        if (m_hasValue)
            kpanic("Attempt to get error from valid result.");

        return std::move(m_error);
    }

    constexpr operator bool() const noexcept { return has_value(); }
};

/********************************************************************************************************************/

#endif /* _X_CPPIMPL_EX_EXPECTED_H__ */

/********************************************************************************************************************/
