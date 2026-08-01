#ifndef FOONIX_TESTS_HARNESS_H
#define FOONIX_TESTS_HARNESS_H

#include <string.h>

#include "test_io.h"

extern int g_checks;
extern int g_failures;

#define CHECK(cond) \
    do { \
        g_checks++; \
        if (!(cond)) { \
            g_failures++; \
            test_io_printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        } \
    } while (0)

#define CHECK_MEM_EQ(a, b, n) CHECK(memcmp((a), (b), (n)) == 0)
#define CHECK_STR_EQ(a, b)    CHECK(test_streq((a), (b)))

/* strcmp() isn't part of this project's string.h -- build string equality
 * out of the two functions that are (strlen/memcmp) instead. */
static inline int test_streq(const char *a, const char *b)
{
    size_t la = strlen(a);
    size_t lb = strlen(b);

    return la == lb && memcmp(a, b, la) == 0;
}

#endif /* FOONIX_TESTS_HARNESS_H */
