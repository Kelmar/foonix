#include <string.h>

#include "harness.h"
#include "tests.h"

static void test_s_strncat_basic(void)
{
    char buf[16] = "Hello, ";
    char *r = _s_strncat(buf, sizeof(buf), "world!", 6);

    CHECK(r == buf);
    CHECK_STR_EQ(buf, "Hello, world!");
}

static void test_s_strncat_truncates_and_terminates(void)
{
    char buf[10] = "Hello, "; /* 7 chars used, room for 2 more + NUL */
    char *r = _s_strncat(buf, sizeof(buf), "world!", 6);

    CHECK(r == buf);
    CHECK(strlen(buf) < sizeof(buf));
    CHECK_STR_EQ(buf, "Hello, wo");
}

static void test_s_strncpy_basic(void)
{
    char buf[16];
    char *r = _s_strncpy(buf, sizeof(buf), "Hi", 2);

    CHECK(r == buf);
    CHECK_STR_EQ(buf, "Hi");
}

static void test_s_strncpy_truncates_and_terminates(void)
{
    /*
     * Regression test: dst[mlen] = '\0' used to be able to write dst[dsz],
     * one byte past the buffer, when the copy exactly filled it.
     */
    char buf[4];
    char *r = _s_strncpy(buf, sizeof(buf), "ABCDEFG", 7);

    CHECK(r == buf);
    CHECK(strlen(buf) < sizeof(buf));
    CHECK(buf[3] == '\0');
}

static void test_strncat_basic(void)
{
    char buf[16] = "Hello, ";
    char *r = strncat(buf, "world!", sizeof(buf));

    CHECK(r == buf);
    CHECK_STR_EQ(buf, "Hello, world!");
}

static void test_strncat_truncates_and_terminates(void)
{
    char buf[10] = "Hello, ";
    char *r = strncat(buf, "world!", sizeof(buf));

    CHECK(r == buf);
    CHECK(strlen(buf) < sizeof(buf));
    CHECK(buf[sizeof(buf) - 1] == '\0');
}

static void test_strncat_dst_not_terminated_within_size(void)
{
    /* No NUL anywhere in buf's claimed `size` -- strncat can't safely
     * find an append point, so it must leave buf untouched rather than
     * scanning/writing past it. */
    char buf[4] = { 'A', 'B', 'C', 'D' };
    char *r = strncat(buf, "X", sizeof(buf));

    CHECK(r == buf);
    CHECK_MEM_EQ(buf, "ABCD", 4);
}

/*
 * strncpy() is tracked in BUGS.md as still broken: it currently searches
 * for an existing NUL in `dst` and appends after it (i.e. it behaves like
 * strncat, not strncpy), and can return NULL, which the standard
 * signature never does. These two encode the CORRECT contract and are
 * expected to fail until that's fixed.
 */
static void test_strncpy_overwrites_from_start(void)
{
    char buf[16] = "XXXXXXXXXXXXXXX";
    char *r = strncpy(buf, "Hi", sizeof(buf));

    CHECK(r == buf);
    CHECK_STR_EQ(buf, "Hi");
}

static void test_strncpy_truncates_without_terminator(void)
{
    char buf[4];
    memset(buf, 'X', sizeof(buf));

    char *r = strncpy(buf, "ABCDEFG", sizeof(buf));

    CHECK(r == buf);
    CHECK_MEM_EQ(buf, "ABCD", 4); /* no room for a NUL; strncpy just fills `size` */
}

static void test_strncpy_pads_remainder_with_nul(void)
{
    /*
     * The standard contract (and the "per Linux man page" comment in
     * strncpy.c) requires ALL `size` bytes of dst to end up written --
     * either copied from src, or explicitly NUL-padded, with no gap.
     */
    char buf[8];
    memset(buf, 'X', sizeof(buf));

    strncpy(buf, "Hi", sizeof(buf));

    CHECK_MEM_EQ(buf, "Hi\0\0\0\0\0\0", 8);
}

void run_bounded_string_tests(void)
{
    test_s_strncat_basic();
    test_s_strncat_truncates_and_terminates();
    test_s_strncpy_basic();
    test_s_strncpy_truncates_and_terminates();
    test_strncat_basic();
    test_strncat_truncates_and_terminates();
    test_strncat_dst_not_terminated_within_size();
    test_strncpy_overwrites_from_start();
    test_strncpy_truncates_without_terminator();
    test_strncpy_pads_remainder_with_nul();
}
