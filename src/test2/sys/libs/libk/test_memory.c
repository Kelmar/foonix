#include <string.h>

#include "harness.h"
#include "tests.h"

static void test_memcpy_basic(void)
{
    char dst[8] = { 0 };
    const char src[] = "ABCDEFG";

    memcpy(dst, src, 7);
    CHECK_MEM_EQ(dst, "ABCDEFG", 7);
}

static void test_memcpy_zero_length(void)
{
    char dst[4] = { 'X', 'X', 'X', 'X' };
    const char src[4] = { 'Y', 'Y', 'Y', 'Y' };

    memcpy(dst, src, 0);
    CHECK_MEM_EQ(dst, "XXXX", 4);
}

static void test_memmove_no_overlap(void)
{
    char buf[8] = "ABCDEFG";
    char dst[8] = { 0 };

    memmove(dst, buf, 7);
    CHECK_MEM_EQ(dst, "ABCDEFG", 7);
}

static void test_memmove_forward_overlap(void)
{
    /* dst < src: shifting "BCDEFG" one position left. */
    char buf[8] = "ABCDEFG";

    memmove(buf, buf + 1, 6);
    CHECK_MEM_EQ(buf, "BCDEFGG", 7);
}

static void test_memmove_backward_overlap(void)
{
    /*
     * dst > src: shifting "ABCDEF" one position right. This is the branch
     * that had the off-by-one bug -- it started at i = size instead of
     * size - 1, reading/writing one element past both buffers and never
     * copying index 0.
     */
    char buf[8] = "ABCDEF\0";

    memmove(buf + 1, buf, 6);
    CHECK_MEM_EQ(buf, "AABCDEF", 7);
}

static void test_memmove_same_pointer(void)
{
    char buf[] = "ABCDEF";

    memmove(buf, buf, 6);
    CHECK_MEM_EQ(buf, "ABCDEF", 6);
}

static void test_memmove_zero_length(void)
{
    char buf[4] = { 'A', 'B', 'C', 'D' };

    memmove(buf + 2, buf, 0);
    CHECK_MEM_EQ(buf, "ABCD", 4);
}

static void test_memset_basic(void)
{
    char buf[6];

    memset(buf, 'Z', sizeof(buf));
    CHECK_MEM_EQ(buf, "ZZZZZZ", 6);
}

static void test_memset_zero_length(void)
{
    char buf[4] = { 'A', 'B', 'C', 'D' };

    memset(buf, 'Z', 0);
    CHECK_MEM_EQ(buf, "ABCD", 4);
}

static void test_memchr_found(void)
{
    const char buf[] = "ABCDEF";

    CHECK(memchr(buf, 'D', sizeof(buf)) == buf + 3);
}

static void test_memchr_not_found(void)
{
    const char buf[] = "ABCDEF";

    CHECK(memchr(buf, 'Z', sizeof(buf) - 1) == NULL);
}

static void test_memchr_finds_embedded_nul(void)
{
    const char buf[] = "AB";

    CHECK(memchr(buf, '\0', sizeof(buf)) == buf + 2);
}

static void test_memcmp_equal(void)
{
    CHECK(memcmp("ABCDEF", "ABCDEF", 6) == 0);
}

static void test_memcmp_orders_by_first_difference(void)
{
    /*
     * Regression test: memcmp.c's "greater than" branch,
     * `else if (b[i] > a[i])`, is logically identical to `a[i] < b[i]`
     * (already ruled out by the preceding if), so it's unreachable and
     * a > b was never detected. Expected to fail until that's fixed.
     */
    CHECK(memcmp("ABC", "ABD", 3) < 0);
    CHECK(memcmp("ABD", "ABC", 3) > 0);

    /* A later byte must not override an earlier decisive difference. */
    CHECK(memcmp("BA", "AZ", 2) > 0);
}

void run_memory_tests(void)
{
    test_memcpy_basic();
    test_memcpy_zero_length();
    test_memmove_no_overlap();
    test_memmove_forward_overlap();
    test_memmove_backward_overlap();
    test_memmove_same_pointer();
    test_memmove_zero_length();
    test_memset_basic();
    test_memset_zero_length();
    test_memchr_found();
    test_memchr_not_found();
    test_memchr_finds_embedded_nul();
    test_memcmp_equal();
    test_memcmp_orders_by_first_difference();
}
