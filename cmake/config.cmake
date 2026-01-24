# =========================================================================
# CMAKE file for loading configuration.
# =========================================================================

set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/config/${CONFIG}.json")

if (NOT EXISTS ${CONFIG_FILE})
  message(FATAL_ERROR "Unknown configuration: ${CONFIG}")
endif()

file(READ "${CONFIG_FILE}" CONFIG_JSON)

string(JSON CONFIG_NAME GET "${CONFIG_JSON}" name)
message("Building configuration: ${CONFIG_NAME}")

string(JSON IS_LINT ERROR_VARIABLE _ GET "${CONFIG_JSON}" isLint)

if (IS_LINT)
  message(FATAL_ERROR "Not building a lint configuraiton.  Remove isLint or set to false.")
endif()

string(JSON USE_BOCHS ERROR_VARIABLE _ GET "${CONFIG_JSON}" useBochs)
string(JSON PLATFORM ERROR_VARIABLE _ GET "${CONFIG_JSON}" platform)

# =========================================================================

# Setup for global BOCHS debugging
if (${USE_BOCHS})
    add_definitions(-D_USE_BOCHS)
endif()

# =========================================================================
