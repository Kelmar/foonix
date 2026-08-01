/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_H__
#define __FOONIX_KERNEL_H__

/********************************************************************************************************************/

#include <stdint.h>

#include <kernel/types.h>

/********************************************************************************************************************/

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
        Unknown = -1,

        /// @brief Successful operation result.
        NoError = 0,
      
        /// @brief Out of memory.
        OutOfMemory = 2,

        /// @brief The requested resource has already been allocated.
        AlreadyInUse = 3,

        /// @brief A requested memory operation needs to be aligned
        NotAligned = 4,

        /// @brief A resource of some sort was not found.
        NotFound = 5,
    };
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_H__ */

/********************************************************************************************************************/
