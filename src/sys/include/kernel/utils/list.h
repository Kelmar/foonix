/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_LIST_H__
#define __FOONIX_KERNEL_LIST_H__

/********************************************************************************************************************/

#include <concepts>
#include <type_traits>

/********************************************************************************************************************/

template <typename T>
concept ListNode = requires(T node)
{
    { node.Next } -> std::convertible_to<T *>;
    { node.Prev } -> std::convertible_to<T *>;
};

/**
 * @brief A basic doubly linked list implementation.
 * @tparam TNode The type that represents a node for the list.
 * 
 * @details This list expects the node to have it's own "Prev" and "Next" pointers that the list is allowed to manipulate.
 * This implementation also does not allocate memory on it's own and requires the caller to maintain deletion and freeing 
 * of items for the list.
 */
template <ListNode TNode>
class List
{
public:
    typedef TNode                   element_type;
    typedef std::remove_cv_t<TNode> value_type;
    typedef size_t                  size_type;
    typedef TNode*                  pointer;
    typedef const TNode *           const_pointer;
    typedef TNode &                 reference;
    typedef const TNode &           const_reference;

private:
    pointer m_first;
    pointer m_last;

    size_t m_count;

public: // Iterators
    template <bool IsConst>
    class IteratorBase
    {
    private:
        // Flag indicating that we are (logically) one past one of the end pointers.
        bool m_onePast;

        TNode *m_current;

        // So a node can be deleted without messing up the iterator itself.
        TNode *m_prev; // Safe previous
        TNode *m_next; // Save next

    public:
        using itoa = std::random_access_iterator_tag;

        using value_type      = std::remove_cv_t<TNode>;
        using difference_type = size_type;
        using pointer         = std::conditional_t<IsConst, const pointer, TNode *>;
        using reference       = std::conditional_t<IsConst, const TNode &, TNode &>;

        explicit IteratorBase(TNode *ptr, bool onePast) noexcept
            : m_onePast(ptr == nullptr || onePast)
            , m_current(ptr)
            , m_prev(ptr ? ptr->Prev : nullptr)
            , m_next(ptr ? ptr->Next : nullptr)
        {
        }

        IteratorBase(const IteratorBase &rhs) noexcept = default;
        IteratorBase(IteratorBase &&rhs) noexcept = default;

        IteratorBase &operator =(const IteratorBase & rhs) = default;
        IteratorBase &operator =(IteratorBase &&) = default;

        reference operator *() const { return *m_current; }
        pointer   operator->() const { return m_current;  }

        template <bool OtherConst>
        constexpr bool operator ==(const IteratorBase<OtherConst> &rhs) const
        {
            return (m_current == rhs.m_current) && (m_onePast == rhs.m_onePast);
        }

        template <bool OtherConst>
        constexpr bool operator !=(const IteratorBase<OtherConst> &rhs) const
        {
            return (m_current != rhs.m_current) || (m_onePast != rhs.m_onePast);
        }

        IteratorBase &operator ++()
        {
            if (m_onePast)
            {
                m_onePast = false;
            }
            else
            {
                if (m_next)
                {
                    m_prev = m_current;
                    m_current = m_next;
                    m_next = m_current->Next;
                }
                else
                    m_onePast = true;
            }

            return *this;
        }

        IteratorBase operator++(int)
        {
            auto result = *this;
            operator ++();
            return result;
        }

        IteratorBase &operator --()
        {
            if (m_onePast)
            {
                m_onePast = false;
            }
            else
            {
                if (m_prev)
                {
                    m_next = m_current;
                    m_current = m_prev;
                    m_prev = m_current->Prev;
                }
                else
                    m_onePast = true;
            }

            return *this;
        }

        IteratorBase operator --(int)
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

public:
    constexpr List() noexcept
        : m_first(nullptr)
        , m_last(nullptr)
        , m_count(0)
    { }

    /// @brief Gets the number of items inserted into the list.
    constexpr size_t Count() const { return m_count; }

    /// @brief Checks to see if the list is empty or not.
    constexpr bool Empty() const { return m_count == 0; }

    constexpr iterator        begin()       { return iterator(m_first, false); }
    constexpr const_iterator  begin() const { return const_iterator(m_first, false); }
    constexpr const_iterator cbegin() const { return const_iterator(m_first, false); }

    constexpr iterator          end()       { return iterator(m_last, true); }
    constexpr const_iterator    end() const { return iterator(m_last, true); }
    constexpr const_iterator   cend() const { return const_iterator(m_last, true); }

    constexpr reverse_iterator        rbegin()       { return reverse_iterator(m_last, false); }
    constexpr const_reverse_iterator  rbegin() const { return const_reverse_iterator(m_last, false); }
    constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(m_last, false); }

    constexpr reverse_iterator          rend()       { return reverse_iterator(m_first, true); }
    constexpr const_reverse_iterator    rend() const { return const_reverse_iterator(m_first, true); }
    constexpr const_reverse_iterator   crend() const { return const_reverse_iterator(m_first, true); }

    /**
     * @brief Preforms an O(n) scan of the list verifying that the node is in fact in this list.
     * @param node The node to search for.
     * @return A boolean indicating if the node is present (true) or false (not found)
     */
    bool Contains(TNode *node)
    {
        for (TNode *p = m_first; p; p = p->Next)
        {
            if (p == node)
                return true;
        }

        return false;
    }

    /// @brief Get a pointer to the first node in the list.
    TNode *Front() { return m_first; }

    /// @brief Get a pointer to the last node in the list.
    TNode *Last() { return m_last; }

    /**
     * @brief Push an item to the front of the list, setting it as the new first item.
     * @param node The node to add.
     */
    void PushFront(TNode *node)
    {
        node->Prev = nullptr;
        node->Next = m_first;

        if (m_first != nullptr)
            m_first->Prev = node;
        else
            m_last = node;

        m_first = node;
        ++m_count;
    }

    /**
     * @brief Push an item to the last value of the list setting it as the new last value.
     * @param node The node to add.
     */
    void PushBack(TNode *node)
    {
        node->Prev = m_last;
        node->Next = nullptr;

        if (m_last != nullptr)
            m_last->Next = node;
        else
            m_first = node;

        m_last = node;
        ++m_count;
    }

    /**
     * @brief Remove the node at the front of the list and return it's pointer.
     * 
     * @return The pointer to the original first item of the list, or a nullptr if the list was empty.
     */
    TNode *PopFront()
    {
        if (!m_first)
            return nullptr;

        TNode *rval = m_first;
        m_first = rval->Next;

        if (!m_first)
            m_last = nullptr;
        else
            m_first->Prev = nullptr;

        --m_count;
        rval->Next = nullptr;
        return rval;
    }

    /**
     * @brief Remove the node at the back of the list and return it's pointer.
     *
     * @return The pointer to the original last item of the list, or a nullptr if the list was empty.
     */
    TNode *PopBack()
    {
        if (!m_last)
            return nullptr;

        TNode *rval = m_last;
        m_last = rval->Prev;

        if (!m_last)
            m_first = nullptr;
        else
            m_last->Next = nullptr;

        --m_count;
        rval->Prev = nullptr;
        return rval;
    }

    /**
     * @brief Remove an item from anywhere in the list.
     * @param node The node to remove.
     */
    void Remove(TNode *node)
    {
        if (node->Prev)
            node->Prev->Next = node->Next;
        else
            m_first = node->Next;

        if (node->Next)
            node->Next->Prev = node->Prev;
        else
            m_last = node->Prev;

        node->Next = nullptr;
        node->Prev = nullptr;

        --m_count;
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_LIST_H__ */

/********************************************************************************************************************/
