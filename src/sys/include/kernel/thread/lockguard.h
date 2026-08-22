/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef FOONIX_KERNEL_THREAD_LOCKGUARD_H__
#define FOONIX_KERNEL_THREAD_LOCKGUARD_H__

/********************************************************************************************************************/

#include <type_traits>

/********************************************************************************************************************/

namespace thread
{
    template <typename T>
    concept BasicLock = requires(T lock)
    {
        { lock.TryLock() } -> std::same_as<bool>;
        { lock.Lock() } -> std::same_as<void>;
        { lock.Unlock() } -> std::same_as<void>;
    };

    /**
    * @brief Utility class for automatically calling Lock/Unlock on a BasicLock concept.
    *
    * Note that this class will first acquire the lock on construction and then release it when it is disposed of.
    */
    template <BasicLock T>
    class LockGuard
    {
    private:
        T &m_lock;

        // Don't allow copy or move semantics for this class.
        LockGuard(const LockGuard &&) = delete;
        LockGuard(LockGuard &&) = delete;

        LockGuard &operator =(const LockGuard &) = delete;
        LockGuard &operator =(LockGuard &&) = delete;

    public:
        constexpr LockGuard(T &lock) noexcept : m_lock(lock) { m_lock.Lock(); }
        ~LockGuard() noexcept { m_lock.Unlock(); }
    };
}

/********************************************************************************************************************/

#endif /* FOONIX_KERNEL_THREAD_LOCKGUARD_H__ */

/********************************************************************************************************************/
