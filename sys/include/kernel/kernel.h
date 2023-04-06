/*************************************************************************/
/*************************************************************************/

#ifndef __FOONIX_KERNEL_H__
#define __FOONIX_KERNEL_H__

/*************************************************************************/

#include <stdint.h>

/// @brief Type for tracking physical address locations.
typedef uintptr_t physical_addr_t;

/// @brief Type for tracking logical address locations.
typedef uintptr_t logical_addr_t;

/// @brief Type for holding a page index.
typedef uint32_t page_index_t;

/*************************************************************************/

namespace Kernel
{
    /// @brief Direction of memory or streams
    enum class Direction
    {
        Up = 0,
        Down = 1
    };

    enum class ErrorCode
    {
        NoError = 0,
        Unknown = -1,
        OutOfMemory = -2,
        AlreadyInUse = -3,
    };
}

/*************************************************************************/

#endif /* __FOONIX_KERNEL_H__ */

/*************************************************************************/
