#include <string.h>

#include "harness.h"
#include "tests.h"

static void test_strlen_basic(void)
{
    CHECK(strlen("") == 0);
    CHECK(strlen("A") == 1);
    CHECK(strlen("Hello, world!") == 13);
}

static void test_strnlen_shorter_than_max(void)
{
    CHECK(strnlen("Hello", 100) == 5);
}

static void test_strnlen_capped_at_max(void)
{
    /* No NUL within the first 3 bytes of "Hello" -> capped at max. */
    CHECK(strnlen("Hello", 3) == 3);
}

static void test_strnlen_zero_max(void)
{
    CHECK(strnlen("Hello", 0) == 0);
}

void run_strlen_tests(void)
{
    test_strlen_basic();
    test_strnlen_shorter_than_max();
    test_strnlen_capped_at_max();
    test_strnlen_zero_max();
}
