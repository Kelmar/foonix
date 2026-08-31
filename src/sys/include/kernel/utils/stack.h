/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_UTILS_STACK_H__
#define __FOONIX_KERNEL_UTILS_STACK_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <type_traits>
#include <concepts>

#include <kernel/thread/lockguard.h>
#include <kernel/thread/spinlock.h>

/********************************************************************************************************************/

template <typename T>
concept Stackable = requires(T node)
{
    { node.Next } -> std::convertible_to<T *>;
};

/********************************************************************************************************************/
/**
 * @brief A basic stack implemented as a singly linked list.
 * 
 * @remarks Nodes are expected to provide their own 'Next' pointer which the stack is allowed to manipulate.
 * 
 * The stack does not allocate or release nodes, the caller is responsible to managing the memory of these objects.
 * 
 * The stack is guarded by a mutex; later versions plan to use a lock free implementation.
 */
template <Stackable TNode>
class Stack
{
public:
    typedef TNode                   element_type;
    typedef std::remove_cv_t<TNode> value_type;
    typedef size_t                  size_type;
    typedef TNode *                 pointer;
    typedef const TNode *           const_pointer;

private:
    // Going to need a proper tagged pointer for the lock free version of this.
    thread::SpinLock m_mutex;

    TNode *m_first;

public:
    constexpr Stack() noexcept
        : m_first(nullptr)
    {
    }

    ~Stack() { }

    /// @brief Check to see if the stack is empty.
    bool Empty() const { return m_first == nullptr; }

    /// @brief Remove an item from the stack, returns null if the stack is empty.
    TNode *Pop()
    {        
        thread::LockGuard l(m_mutex);

        pointer result = m_first;

        if (m_first)
        {
            m_first = m_first->Next;
            result->Next = nullptr;
        }

        return result;

#if 0
        
        for (;;)
        {
            pointer expected = m_first;

            if (!expected)
                return nullptr;

            //pointer result = atomic::cmpset(&m_first, expected, m_first->Next);
            bool result = std::atomic_compare_exchange_weak(&m_first, expected, m_first->Next);

            if (result)
            {
                result->Next = nullptr;
                return result;
            }
        }
#endif
    }

    /// @brief Push an item to the front of the stack.
    void Push(TNode *node)
    {
        if (!node)
            return;

        thread::LockGuard l(m_mutex);

        node->Next = m_first;
        m_first = node;

#if 0
        bool swapped = false;

        while (!swapped)
        {
            pointer expected = m_first;
            node->Next = expected;
            //swapped = atomic::cmpset(&m_first, expected, node);
            swapped = std::atomic_compare_exchange_weak(&m_first, expected, node);
        }
#endif
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_UTILS_STACK_H__ */

/********************************************************************************************************************/

