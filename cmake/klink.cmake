# =========================================================================
# Handles dealing with finding C runtime based object files
# =========================================================================

# Ask compiler for the specified file.
function(find_crt_object FILE dest)
    execute_process(
        COMMAND
            "${CMAKE_CXX_COMPILER}" ${TARGET_FLAGS} -print-file-name=${FILE}
        OUTPUT_VARIABLE TMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    message("COMMAND: ${CMAKE_CXX_COMPILER} ${TARGET_FLAGS} -print-file-name=${FILE}")
    message("RESULT: ${TMP}")

    # Clang will return the filename w/o a path if it can't find the object.
    #
    # Which isn't what we want; we verify the file's existance so we'll fail
    # the next check.
    if (NOT EXISTS ${TMP})
        message("Cant find file ${TMP}")
        unset(TMP)
    endif()

    set(${dest} "${TMP}")
    return (PROPAGATE ${dest})
endfunction()

# =========================================================================

#
# First check to see if the included host specifics haven't already defined
# a location for their CRTBEGIN_OBJ and CRTEND_OBJ values.
#
# If not found, we ask the compiler to tell us where it is.
#

if (NOT CRTBEGIN_OBJ)
    find_crt_object("crtbegin.o" CRTBEGIN_OBJ)

    if (NOT CRTBEGIN_OBJ)
        message(FATAL_ERROR "Unable to find ${PLATFORM} crtbegin.o")
    else()
        message(STATUS "Found: ${CRTBEGIN_OBJ}")
    endif()
endif()

if (NOT CRTEND_OBJ)
    find_crt_object("crtend.o" CRTEND_OBJ)

    if (NOT CRTEND_OBJ)
        message(FATAL_ERROR "Uanble to find ${PLATFORM} crtend.o")
    else()
        message(STATUS "Found: ${CRTEND_OBJ}")
    endif()
endif()

# =========================================================================

# We need some of these objects to be in a very specific order to get the
# C++ static ctors functioning properoy.

set(CMAKE_CXX_LINK_EXECUTABLE
    "<CMAKE_CXX_COMPILER> <CMAKE_CXX_LINK_FLAGS> <FLAGS>\
    <LINK_FLAGS>\
    ${CRTBEGIN_OBJ}\
    <OBJECTS> <LINK_LIBRARIES>\
    ${CRTEND_OBJ}\
    -o <TARGET>")

# =========================================================================
