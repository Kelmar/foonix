
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
