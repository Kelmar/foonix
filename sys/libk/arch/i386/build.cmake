file(GLOB string_asms arch/i386/string/*.S)

foreach(asm_file ${string_asms})
    #message("ASM: ${asm_file}")
    get_filename_component(file_name ${asm_file} NAME_WLE)
    set(file_name ".*\\/${file_name}\\.c")
    #message("REPLACE: ${file_name}")
    list(FILTER string_srcs EXCLUDE REGEX ${file_name})
    list(APPEND string_srcs ${asm_file})
endforeach()

