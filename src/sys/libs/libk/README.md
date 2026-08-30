A small set of standard C library functions.

This is not a complete set of POSIX compatible C runtime functions.

It does however feature the ability to override some functions with assembly optimized 
versions if it makes sense to do so for a given platform.

E.g. the string functions on x86 platforms.
