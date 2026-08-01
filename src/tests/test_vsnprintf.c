#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "tests.h"

static void test_literal_text(void)
{
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "Hello, world!");

    CHECK(n == 13);
    CHECK_STR_EQ(buf, "Hello, world!");
}

static void test_percent_literal(void)
{
    char buf[8];

    snprintf(buf, sizeof(buf), "100%%");
    CHECK_STR_EQ(buf, "100%");
}

static void test_char(void)
{
    char buf[8];

    snprintf(buf, sizeof(buf), "[%c]", 'Q');
    CHECK_STR_EQ(buf, "[Q]");
}

static void test_string(void)
{
    char buf[16];

    snprintf(buf, sizeof(buf), "<%s>", "abc");
    CHECK_STR_EQ(buf, "<abc>");
}

static void test_signed_decimal(void)
{
    char buf[16];

    snprintf(buf, sizeof(buf), "%d", 42);
    CHECK_STR_EQ(buf, "42");

    snprintf(buf, sizeof(buf), "%d", -42);
    CHECK_STR_EQ(buf, "-42");
}

static void test_unsigned_hex_case(void)
{
    char buf[16];

    snprintf(buf, sizeof(buf), "%x", 0xBEEFu);
    CHECK_STR_EQ(buf, "beef");

    snprintf(buf, sizeof(buf), "%X", 0xBEEFu);
    CHECK_STR_EQ(buf, "BEEF");
}

static void test_pointer(void)
{
    char buf[16];

    snprintf(buf, sizeof(buf), "%p", (void *)(uintptr_t)0xBEEF);
    CHECK_STR_EQ(buf, "0x0000BEEF");
}

static void test_exact_fit_no_overflow(void)
{
    /* Buffer capacity exactly matches the formatted length. */
    char buf[6];
    int n = snprintf(buf, sizeof(buf), "%s", "Hello");

    CHECK(n == 5);
    CHECK_STR_EQ(buf, "Hello");
}

static void test_truncates_long_string_safely(void)
{
    char buf[4];

    snprintf(buf, sizeof(buf), "%s", "Hello, world!");

    CHECK(strlen(buf) < sizeof(buf));
    CHECK(buf[sizeof(buf) - 1] == '\0');
}

static void test_negative_number_fills_buffer_exactly(void)
{
    /*
     * Regression test: a sign character landing exactly on the last free
     * byte used to leave s == slen when falling into the shared `string:`
     * label, underflowing `slen - s - 1` and allowing an unbounded write
     * via memset()/_s_strncat(). Buffer sized so the sign character is the
     * last byte that can possibly fit before the width padding kicks in.
     */
    char buf[4];

    snprintf(buf, sizeof(buf), "%5d", -3);

    CHECK(strlen(buf) < sizeof(buf));
    CHECK(buf[sizeof(buf) - 1] == '\0');
}

static void test_buffer_full_before_specifier(void)
{
    char buf[4];

    snprintf(buf, sizeof(buf), "ABC%s", "more text");

    CHECK(strlen(buf) < sizeof(buf));
    CHECK(buf[sizeof(buf) - 1] == '\0');
    CHECK_MEM_EQ(buf, "ABC", 3);
}

static void test_zero_size_buffer(void)
{
    /*
     * Not currently guarded against: the exit: clamp computes
     * slen - 1 on slen == 0, which underflows, so sbuf[0] still gets
     * written even though the caller claimed zero usable bytes. Not yet
     * in BUGS.md as its own item -- flagged here so a fix shows up as a
     * newly-passing test. Included for the sanitizer's benefit as much as
     * the assertion itself.
     */
    char buf[1] = { 'X' };

    snprintf(buf, 0, "Hello");
    CHECK(buf[0] == 'X');
}

void run_vsnprintf_tests(void)
{
    test_literal_text();
    test_percent_literal();
    test_char();
    test_string();
    test_signed_decimal();
    test_unsigned_hex_case();
    test_pointer();
    test_exact_fit_no_overflow();
    test_truncates_long_string_safely();
    test_negative_number_fills_buffer_exactly();
    test_buffer_full_before_specifier();
    test_zero_size_buffer();
}
