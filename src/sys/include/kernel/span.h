/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_SPAN_H__
#define __FOONIX_KERNEL_SPAN_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <iterator>
#include <type_traits>
#include <utility>

#include <kernel/flow.h>

/********************************************************************************************************************/

namespace util
{
    // Similar to std::span, but not exactly the same...

    static constexpr size_t default_stride = (size_t)(-1);

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

        template <bool IsConst>
        class IteratorBase
        {
        private:
            using container_type = std::conditional_t<IsConst, const span&, span&>;

            container_type m_container;
            size_t m_index;

            template <bool> friend class IteratorBase;

        public:
            using value_type      = std::remove_cv_t<T>;
            using difference_type = size_type;
            using pointer         = std::conditional_t<IsConst, const T*, T*>;
            using reference       = std::conditional_t<IsConst, const T&, T&>;

            explicit IteratorBase(container_type container, size_t index = 0) noexcept
                : m_container(container)
                , m_index(index)
            {
            }

            IteratorBase(const IteratorBase &rhs) noexcept = default;
            IteratorBase(IteratorBase &&rhs) noexcept = default;

            // Implicit iterator -> const_iterator conversion (mirrors std:: container iterators).
            template <bool WasConst, typename = std::enable_if_t<IsConst && !WasConst>>
            IteratorBase(const IteratorBase<WasConst> &rhs) noexcept
                : m_container(rhs.m_container)
                , m_index(rhs.m_index)
            {
            }

            IteratorBase &operator =(const IteratorBase &rhs) noexcept = default;
            IteratorBase &operator =(IteratorBase &&rhs) noexcept = default;

            reference operator * () const { return m_container.at(m_index); }
            pointer   operator ->() const { return m_container.fetch(m_index); }

            template <bool OtherConst>
            constexpr bool operator ==(const IteratorBase<OtherConst> &rhs) const
            {
                return (m_container.m_data == rhs.m_container.m_data) && (m_index == rhs.m_index);
            }

            template <bool OtherConst>
            constexpr bool operator !=(const IteratorBase<OtherConst> &rhs) const
            {
                return !(*this == rhs);
            }

            IteratorBase &operator ++()
            {
                ++m_index;
                return *this;
            }

            IteratorBase  operator ++(int)
            {
                auto result = *this;
                operator ++();
                return result;
            }

            IteratorBase &operator --()
            {
                --m_index; return *this;
            }

            IteratorBase  operator --(int)
            {
                auto result = *this;
                operator --();
                return result;
            }
        };

        typedef IteratorBase<false> iterator;
        typedef IteratorBase<true>  const_iterator;

        typedef std::reverse_iterator<iterator>       reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    private:
        pointer   m_data;
        size_type m_size;   // Number of elements
        size_type m_stride; // Element stride (defaults to sizeof(T))

        constexpr const_pointer fetch(size_type pos) const
        {
            if (pos >= m_size)
                kpanic("Attempt to index span outside of max range.");

            uintptr_t i = reinterpret_cast<uintptr_t>(m_data);
            i += m_stride * pos;
            return reinterpret_cast<const_pointer>(i);
        }

        constexpr pointer fetch(size_type pos)
        {
            const_pointer p = std::as_const(*this).fetch(pos);
            return const_cast<pointer>(p);
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

        constexpr iterator             begin()       { return iterator(*this, 0); }
        constexpr const_iterator       begin() const { return const_iterator(*this, 0); }
        constexpr const_iterator      cbegin() const { return const_iterator(*this, 0); }

        constexpr reverse_iterator        rbegin()       { return reverse_iterator(end()); }
        constexpr const_reverse_iterator  rbegin() const { return const_reverse_iterator(end()); }
        constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }

        constexpr iterator             end()       { return iterator(*this, m_size); }
        constexpr const_iterator       end() const { return const_iterator(*this, m_size); }
        constexpr const_iterator      cend() const { return const_iterator(*this, m_size); }

        constexpr reverse_iterator        rend()       { return reverse_iterator(begin()); }
        constexpr const_reverse_iterator  rend() const { return const_reverse_iterator(begin()); }
        constexpr const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

        constexpr reference       first()       { return at(0); }
        constexpr const_reference first() const { return at(0); }

        constexpr reference       last()       { return at(m_size - 1); }
        constexpr const_reference last() const { return at(m_size - 1); }

        constexpr reference       at(size_type pos)       { return *fetch(pos); }
        constexpr const_reference at(size_type pos) const { return *fetch(pos); }
    
        constexpr reference       operator[](size_type pos)       { return at(pos); }
        constexpr const_reference operator[](size_type pos) const { return at(pos); }

        // Non-standard extension to quickly get pointer to item.
        constexpr pointer       pointer_to(size_type pos)       { return fetch(pos); }
        constexpr const_pointer pointer_to(size_type pos) const { return fetch(pos); }

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
