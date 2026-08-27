/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_THREAD_SPINLOCK_H__
#define __FOONIX_KERNEL_THREAD_SPINLOCK_H__

/********************************************************************************************************************/

#include "atomic.h"
#include "cpu.h"

namespace thread
{
    /// @brief A basic spin lock that can be locked, and unlocked.
    class SpinLock
    {
    private:
        uint8_t m_value;

    public:
        constexpr SpinLock() noexcept
            : m_value(0)
        {
        }

        ~SpinLock() { }

        /**
         * @brief Attempts to acquire the lock.
         * @return True if the lock was successfully acquired, false if not.
         */
        bool TryLock() noexcept { return atomic::xchg(&m_value, 1) == 0; }

        /**
         * @brief Spins to acquire a lock.
         * @remarks This will spin indefinitely to acquire the lock.
         */
        void Lock() noexcept
        {
            while (!TryLock())
                cpu::pause();
        }

        /// @brief Releases the lock.
        void Unlock() noexcept
        {
            atomic::store(&m_value, 0);
        }
    };
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_THREAD_SPINLOCK_H__ */

/********************************************************************************************************************/
