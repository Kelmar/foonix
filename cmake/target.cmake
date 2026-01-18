# =========================================================================
# Processes user selected target platform options
# =========================================================================

function(get_default_target)
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" -print-target-triple
        OUTPUT_VARIABLE TARGET_TRIPLE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    string(REPLACE "-" ";" TMP_TARGET ${TARGET_TRIPLE})
    list(POP_FRONT TMP_TARGET TARGET_HOST)

    return(PROPAGATE TARGET_HOST)
endfunction()

# =========================================================================

if (PLATFORM)
  set(TARGET_HOST "${PLATFORM}")
else()
  get_default_target()

  message(STATUS "Platform not set defaulting to: ${TARGET_HOST}")
  set(PLATFORM "${TARGET_HOST}")
endif()

set(TARGET_INCLUDE "${CMAKE_SOURCE_DIR}/cmake/arch/${PLATFORM}.cmake")

if (NOT EXISTS "${TARGET_INCLUDE}")
  message(FATAL_ERROR "Unknown configured platform: ${PLATFORM}")
endif()

# =========================================================================

message(STATUS "Building for host: ${PLATFORM}")
include ("${TARGET_INCLUDE}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${TARGET_FLAGS}")

# =========================================================================
