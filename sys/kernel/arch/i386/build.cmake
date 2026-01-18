file(GLOB_RECURSE dev_files "${HOST_DIR}/dev/*.cpp")
file(GLOB_RECURSE x86_cpp "${HOST_DIR}/i386/*.cpp")
file(GLOB_RECURSE x86_asm "${HOST_DIR}/i386/*.S")
file(GLOB base_asm "${HOST_DIR}/*.S")

# Some one off files....
list(APPEND x86_asm "${HOST_DIR}/i386/vectors.s")

list(APPEND SOURCES "${dev_files}")
list(APPEND SOURCES "${x86_cpp}")
list(APPEND SOURCES "${x86_asm}")
list(APPEND SOURCES "${base_asm}")

include_directories("${HOST_DIR}/include")

list(APPEND LINK_EXTRA "-T{HOST_DIR}/linker.ld")

