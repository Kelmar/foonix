/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_SPAN_H__
#define __FOONIX_KERNEL_SPAN_H__

/********************************************************************************************************************/

#include <iterator>

#include <kernel/flow.h>

/********************************************************************************************************************/

namespace util
{
    // Similar to std::span, but not exactly the same...

    constexpr size_t default_stride = (size_t)(-1);

    template <typename T>
    class span
    {
    public:
        typedef T                 element_type;
        typedef std::remove_cv<T> value_type;
        typedef size_t            size_type;
        typedef T*                pointer;
        typedef const T*          const_pointer;
        typedef T&                reference;
        typedef const T&          const_reference;

    public: // Iterators
        class Iterator
        {
        private:
            const span &m_container;
            int m_index;

        public:
            using value_type      = std::remove_cv<T>;
            using difference_type = size_type;
            using pointer         = T*;
            using reference       = T&;

            explicit Iterator(const span &container, int index = 0) noexcept
                : m_container(container)
                , m_index(index)
            {
            }

            Iterator(const Iterator &rhs) noexcept = default;
            Iterator(Iterator &&rhs) noexcept = default;

            Iterator &operator =(const Iterator &rhs) noexcept = default;
            Iterator &operator =(Iterator &&rhs) noexcept = default;

            //reference       operator * ()       { return m_container.at(m_index); }
            const_reference operator * () const { return m_container.at(m_index); }
            //pointer         operator ->()       { return m_container.fetch(m_index); }
            const_pointer   operator ->() const { return m_container.fetch(m_index); }

            constexpr bool operator ==(const Iterator &rhs) const
            {
                return (m_container.m_data == rhs.m_container.m_data) && (m_index == rhs.m_index);
            }

            constexpr bool operator !=(const Iterator &rhs) const
            {
                return (m_container.m_data != rhs.m_container.m_data) || (m_index != rhs.m_index);
            }

            Iterator &operator ++()    { ++m_index; return *this; }
            Iterator  operator ++(int) { auto tmp = *this; ++(*this); return tmp; }

            Iterator &operator --()    { --m_index; return *this; }
            Iterator  operator --(int) { auto tmp = *this; --(*this); return tmp; }
        };

        friend class Iterator;

        typedef Iterator       iterator;
        typedef const Iterator const_iterator;

        typedef std::reverse_iterator<Iterator>       reverse_iterator;
        typedef const std::reverse_iterator<Iterator> const_reverse_iterator;

    private:
        pointer   m_data;
        size_type m_size;   // Number of elements
        size_type m_stride; // Element stride (defaults to sizeof(T))

        constexpr const_pointer fetch(size_type pos) const
        {
            if (pos > m_size)
                kpanic("Attempt to index span outside of max range.");

            uintptr_t i = reinterpret_cast<uintptr_t>(m_data);
            i += m_stride * pos;
            return reinterpret_cast<const_pointer>(i);
        }

        constexpr pointer fetch(size_type pos)
        {
            const_pointer p = fetch(pos);
            return reinterpret_cast<pointer>(p);
        }

    public:
        span() noexcept
            : m_data(nullptr)
            , m_size(0)
            , m_stride(sizeof(T))
        {
        }

        span(pointer first, size_t count, size_t stride = default_stride)
            : m_data(first)
            , m_size(count)
            , m_stride(stride == default_stride ? sizeof(T) : stride)
        {
        }

        span(const span &rhs) noexcept = default;
        span(span &&rhs) noexcept = default;

        ~span() = default;

        constexpr bool empty() const { return m_size == 0; }

        /**
         * @brief Fetches the number of items in the span.
         */
        constexpr size_type size() const { return m_size; }

        /**
         * @brief Fetches the size of the span in bytes.
         *
         * @remarks This may be different than size() * sizeof(T) if the stride is set to a larger size than the element sizes.
         */
        constexpr size_type size_bytes() const { return m_size * m_stride; }

        constexpr iterator                 begin() const { return iterator(*this, 0); }
        constexpr const_iterator          cbegin() const { return iterator(*this, 0); }
        constexpr reverse_iterator        rbegin() const { return std::reverse_iterator(begin()); }
        constexpr const_reverse_iterator crbegin() const { return std::reverse_iterator(cbegin()); }

        constexpr iterator                 end() const { return iterator(*this, m_size); }
        constexpr const_iterator          cend() const { return iterator(*this, m_size); }
        constexpr reverse_iterator        rend() const { return std::reverse_iterator(end()); }
        constexpr const_reverse_iterator crend() const { return std::reverse_iterator(cend()); }

        constexpr reference       first()       { return at(0); }
        constexpr const_reference first() const { return at(0); }

        constexpr reference       last()       { return at(m_size - 1); }
        constexpr const_reference last() const { return at(m_size - 1); }

        constexpr reference       at(size_type pos)       { return *fetch(pos); }
        constexpr const_reference at(size_type pos) const { return *fetch(pos); }
    
        constexpr reference       operator[](size_type pos)       { return at(pos); }
        constexpr const_reference operator[](size_type pos) const { return at(pos); }

        constexpr pointer       data()       { return m_data; }
        constexpr const_pointer data() const { return m_data; }

        span &operator =(const span &rhs) noexcept = default;
        span &operator =(span &&rhs)      noexcept = default;

        constexpr operator T*      ()       noexcept { return m_data; }
        constexpr operator const T*() const noexcept { return m_data; }
    };
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_SPAN_H__ */

/********************************************************************************************************************/
