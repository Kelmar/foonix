/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _LIBKPP_CPPIMPL_REV_ITERATOR_H__
#define _LIBKPP_CPPIMPL_REV_ITERATOR_H__ 1

/********************************************************************************************************************/

template <class TIterator>
class reverse_iterator
{
protected:
    TIterator m_current = TIterator();

public:
    reverse_iterator() = default;

    constexpr explicit reverse_iterator(TIterator iterator) : m_current(iterator) { }

    constexpr decltype(auto) operator *() const
    {
        auto tmp = m_current;
        --tmp;
        return *tmp;
    }

    constexpr reverse_iterator &operator ++()    { --m_current; return *this; }
    constexpr reverse_iterator &operator ++(int) { auto tmp = *this; ++(*this); return tmp; }

    constexpr reverse_iterator &operator --()    { ++m_current; return *this; }
    constexpr reverse_iterator &operator --(int) { auto tmp = *this; --(*this); return tmp; }

    constexpr TIterator base() const { return m_current; }
};

/********************************************************************************************************************/

#endif /* _LIBKPP_CPPIMPL_REV_ITERATOR_H__ */

/********************************************************************************************************************/
