/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_PAGE_PTR_H__
#define __FOONIX_KERNEL_PAGE_PTR_H__

/********************************************************************************************************************/

#include <stdio.h>
#include <stdint.h>

#include <cstddef>
#include <type_traits>
#include <concepts>

#include <kernel/types.h>

#include "cpu.h"

/********************************************************************************************************************/

/**
 * @brief A tagged pointer that points to one or more pages.
 *
 * @details Represents a pointer that points to a page.  It stores additional metadata
 * in the area that is masked off by the paging system to include some useful details.
 *
 * PagePointers can be flagged as physical or virtual, and for physical pages may contain
 * an order value, indicating what order in the buddy list they were split off from.
 *
 * The goal here is to make it so that virtual and physical pointers do not equate to
 * each other, additionally, split/merged physical pointers would also participate in
 * proper comparison operations.
 */
class PagePointer
{
    static_assert(cpu::PageShift >= 5, "CPU needs to have at least 5 bits in masked area.");

private:
    static constexpr uintptr_t VirtualFlag = 0x0010;

    // Mask for getting order value.
    static constexpr uintptr_t OrderMask = 0x000F;

    uintptr_t m_ptr;

    constexpr PagePointer(uintptr_t p) noexcept : m_ptr(p & cpu::PageMask) { }

public:
    constexpr PagePointer() noexcept : m_ptr(0) { }

    constexpr PagePointer(const PagePointer &rhs) noexcept = default;
    constexpr PagePointer(PagePointer &&rhs) noexcept = default;

    /// @brief Construct a PagePointer from a physical page address.
    static inline
    PagePointer FromPhysical(paddr_t addr, uint8_t order = 0) noexcept
    {
        PagePointer result(addr);
        result.m_ptr |= static_cast<uintptr_t>(order & OrderMask);
        return result;
    }

    /// @brief Construct a PagePointer from a virtual page address.
    static inline
    PagePointer FromVirtual (vaddr_t addr) noexcept
    {
        PagePointer result(addr);
        result.m_ptr |= VirtualFlag;
        return result;
    }

    inline
    constexpr PagePointer& operator =(const PagePointer &rhs) noexcept = default;

    inline
    constexpr PagePointer& operator =(PagePointer &&rhs) noexcept = default;

    /// @brief Clear the pointer value setting it to nullptr.
    inline void Clear() { m_ptr = 0; }

    /// @brief Get the packed ordering value for physical page pointers.
    inline
    constexpr uint8_t Order() { return static_cast<int>(m_ptr & OrderMask); }

    /// @brief Check if a pointer is virtual or not.
    inline
    constexpr bool IsVirtual() const { return (m_ptr & VirtualFlag) != 0; }

    /// @brief Check if a pointer is physical or not.
    inline
    constexpr bool IsPhysical() const { return (m_ptr & VirtualFlag) == 0; }

    /// @brief Tests if pointer is null, while ignoring packed bits.
    inline
    constexpr bool operator ==(const std::nullptr_t &) const
    {
        return (m_ptr & cpu::PageMask) == 0;
    }

    /// @brief Tests if pointer is NOT null, while ignoring packed bits.
    inline
    constexpr bool operator !=(const std::nullptr_t &) const
    {
        return (m_ptr & cpu::PageMask) != 0;
    }

    /**
     * @brief Tests if two PagePointers are equivalent, taking packed bit rules into account.
     *
     * This means that even if two pointers have the same upper bits relative to their address mask,
     * they may still be not equal if one is virtual and the other is physical.
     */
    inline
    constexpr bool operator ==(const PagePointer &rhs) const = default;

    /**
     * @brief Tests if two PagePointers are NOT equivalent, taking packed bit rules into account.
     *
     * This means that even if two pointers have the same upper bits relative to their address mask,
     * they may still be not equal if one is virtual and the other is physical.
     */
    inline
    constexpr bool operator !=(const PagePointer &rhs) const = default;

    /// @brief Tests if pointer is null, while ignoring packed bits.
    inline
    operator bool() const { return *this != nullptr; }

    /// @brief Implicit conversion of PagePointer to uintptr_t
    inline
    constexpr operator uintptr_t() const { return m_ptr & cpu::PageMask; }

    template <typename T>
    requires std::is_pointer_v<T>
    inline
    constexpr T PointerTo() const
    {
        return reinterpret_cast<T>(m_ptr & cpu::PageMask);
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_PAGE_PTR_H__ */

/********************************************************************************************************************/
