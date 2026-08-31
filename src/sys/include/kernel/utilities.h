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
     * Returns a Kernel::ErrorCode if an integer cannot be parsed. Otherwise the value parsed is returned.
     */ 
    std::expected<int, Kernel::ErrorCode> parseInt(const std::string_view &);
}

/********************************************************************************************************************/

#include <kernel/utils/align.h>
#include <kernel/utils/flags.h>

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_UTILITIES_H__ */

/********************************************************************************************************************/
