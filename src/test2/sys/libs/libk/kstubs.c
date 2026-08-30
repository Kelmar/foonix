/*
 * Stubs for some kernel functions we call from our not quite standard C library.
 */
 
#include <stdlib.h>
#include <stdio.h>

__attribute__((noreturn))
void kpanic(const char *message)
{
    printf("KPANIC: %s\n", message);
    abort();
}

__attribute__((noreturn))
void kassert(const char *test, const char *reason, int line, const char *file, const char *function)
{
    printf("KASSERT FAILED: %s\n  expression: %s\n  at %s:%d (%s)\n", reason, test, file, line, function);
    abort();
}
