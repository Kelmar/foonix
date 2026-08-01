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

    string(REPLACE "-" ";" TRIP ${TARGET_TRIPLE})
    list(POP_FRONT TRIP PLATFORM)

    return(PROPAGATE PLATFORM)
endfunction()

# =========================================================================

if (NOT DEFINED PLATFORM)
  get_default_target()

  message(STATUS "Platform not set defaulting to: ${PLATFORM}")
endif()

if (PLATFORM MATCHES "i[4-9]86")
  # Be forgiving about specifying the i386 target.
  set(PLATFORM "i386")
endif()

set(TARGET_INCLUDE "${CMAKE_SOURCE_DIR}/cmake/arch/${PLATFORM}.cmake")

if (NOT EXISTS "${TARGET_INCLUDE}")
  message(FATAL_ERROR "Unknown configured platform: ${PLATFORM}")
endif()

# =========================================================================

message(STATUS "Building for host: ${PLATFORM}")
include ("${TARGET_INCLUDE}")

list(JOIN TARGET_FLAGS " " TF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TF}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TF}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${TF}")

unset(TF)

# =========================================================================
