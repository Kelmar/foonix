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
 * PagePointers can be flagged as physical or virtual, and may contain an order value,
 * indicating what order in the buddy list they were split off from.  This effectively
 * means that a PagePointer can represent several pages.
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

    /**
     * @brief Mask for getting order value.
     *
     * @details The order value is the buddy allocation order.  Effectively this gives
     * the number of pages in powers of 2.
     *
     * For instance, an order 0 is 1 page, order 1 = 2 pages, order 2 = 4 pages, and so on.
     * This means we can represent a range of upto 32K pages in one pointer.
     */
    static constexpr uintptr_t OrderMask = 0x000F;

private:
    uintptr_t m_ptr;

    constexpr PagePointer(uintptr_t p, uint8_t order) noexcept
        : m_ptr(p & cpu::PageMask)
    {
        m_ptr |= static_cast<uintptr_t>(order & OrderMask);
    }

public:
    constexpr PagePointer() noexcept : m_ptr(0) { }

    constexpr PagePointer(const PagePointer &rhs) noexcept = default;
    constexpr PagePointer(PagePointer &&rhs) noexcept = default;

    /// @brief Construct a PagePointer from a physical page address.
    static inline
    PagePointer FromPhysical(paddr_t addr, uint8_t order = 0) noexcept
    {
        PagePointer result(addr, order);
        return result;
    }

    /// @brief Construct a PagePointer from a virtual page address.
    static inline
    PagePointer FromVirtual(vaddr_t addr, uint8_t order = 0) noexcept
    {
        PagePointer result(addr, order);
        result.m_ptr |= VirtualFlag;
        return result;
    }

    inline
    constexpr PagePointer& operator =(const PagePointer &rhs) noexcept = default;

    inline
    constexpr PagePointer& operator =(PagePointer &&rhs) noexcept = default;

    /// @brief Clear the pointer value setting it to nullptr.
    inline void Clear() { m_ptr = 0; }

    /// @brief Check if the pointer is null or not.
    inline
    constexpr bool IsNull() const { return (m_ptr & cpu::PageMask) == 0; }

    /// @brief Get the pointer part of the value, stripping off the metadata bits.
    inline
    constexpr uintptr_t Value() const { return m_ptr & cpu::PageMask; }

    /// @brief Get the packed ordering value for physical page pointers.
    inline
    constexpr uint8_t Order() const { return static_cast<int>(m_ptr & OrderMask); }

    /**
     * @brief Get the number of pages that this page pointer represents.
     *
     * @remarks For null pointers, this value will return 0.
     */
    inline
    constexpr size_t Count() const { return IsNull() ? 0 : (1 << Order()); }

    /// @brief Check if a pointer is virtual or not.
    inline
    constexpr bool IsVirtual() const { return (m_ptr & VirtualFlag) != 0; }

    /// @brief Check if a pointer is physical or not.
    inline
    constexpr bool IsPhysical() const { return (m_ptr & VirtualFlag) == 0; }

    /// @brief Tests if pointer is null, while ignoring packed bits.
    inline
    constexpr bool operator ==(const std::nullptr_t &) const { return IsNull(); }

    /// @brief Tests if pointer is NOT null, while ignoring packed bits.
    inline
    constexpr bool operator !=(const std::nullptr_t &) const { return !IsNull(); }

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
    operator bool() const { return !IsNull(); }

    /// @brief Implicit conversion of PagePointer to uintptr_t
    inline
    constexpr operator uintptr_t() const { return Value(); }

    /// @brief Convert to a pointer of the given type.
    template <typename T>
    requires std::is_pointer_v<T>
    inline
    constexpr T PointerTo() const
    {
        return reinterpret_cast<T>(Value());
    }
};

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_PAGE_PTR_H__ */

/********************************************************************************************************************/
