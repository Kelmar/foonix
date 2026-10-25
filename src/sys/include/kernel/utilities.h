/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_UTILITIES_H__
#define __FOONIX_KERNEL_UTILITIES_H__

/********************************************************************************************************************/

#include <kernel/kernel.h>
#include <kernel/types.h>

#include <type_traits>
#include <concepts>
#include <expected>
#include <string_view>

#include <kernel/utils/span.h>

/********************************************************************************************************************/

namespace util
{
    /**
     * @brief Parse a string_view for an integer value.
     *
     * Supports handling of 0x and $ prefixes for hexidecimal numbers.
     *
     * Returns a kernel::ErrorCode if an integer cannot be parsed. Otherwise the value parsed is returned.
     */ 
    std::expected<uintptr_t, kernel::ErrorCode> parseInt(const std::string_view &);

    /**
     * @brief Return the integer base raised to the power of exponent.
     */
    constexpr uint32_t ipow(uint32_t base, unsigned int exponent)
    {
        uint32_t result = 1;

        for (;exponent > 0; exponent >>= 1, base *= base)
        {
            if (exponent & 1)
                result *= base;
        }
        
        return result;
    }

    /**
     * @brief Return the integer base raised to the power of exponent.
     */
    constexpr uint64_t ipow(uint64_t base, unsigned int exponent)
    {
        uint64_t result = 1;

        for (;exponent > 0; exponent >>= 1, base *= base)
        {
            if (exponent & 1)
                result *= base;
        }
        
        return result;
    }
}

/********************************************************************************************************************/

#include <kernel/utils/align.h>
#include <kernel/utils/flags.h>

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_UTILITIES_H__ */

/********************************************************************************************************************/
