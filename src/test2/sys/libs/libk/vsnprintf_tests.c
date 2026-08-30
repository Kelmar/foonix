#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tests.h"

TEST_CASE("snprintf() copies basic literal string")
{
    char buf[32];
    int n = PREFIX_(snprintf)(buf, sizeof(buf), "Hello, world!");

    CHECK(n == 13);
    CHECK_STR_EQ(buf, "Hello, world!");
}

TEST_CASE("snprintf() handles percent escape")
{
    char buf[8];

    PREFIX_(snprintf)(buf, sizeof(buf), "100%%");
    CHECK_STR_EQ(buf, "100%");
}

TEST_CASE("snprintf() character format")
{
    char buf[8];

    PREFIX_(snprintf)(buf, sizeof(buf), "[%c]", 'Q');
    CHECK_STR_EQ(buf, "[Q]");
}

TEST_CASE("snprintf() string format")
{
    char buf[16];

    PREFIX_(snprintf)(buf, sizeof(buf), "<%s>", "abc");
    CHECK_STR_EQ(buf, "<abc>");
}

TEST_CASE("snprintf() signed decimal format")
{
    char buf[16];

    PREFIX_(snprintf)(buf, sizeof(buf), "%d", 42);
    CHECK_STR_EQ(buf, "42");

    PREFIX_(snprintf)(buf, sizeof(buf), "%d", -42);
    CHECK_STR_EQ(buf, "-42");
}

TEST_CASE("snprintf() unsigned hex format")
{
    char buf[16];

    PREFIX_(snprintf)(buf, sizeof(buf), "%x", 0xBEEFu);
    CHECK_STR_EQ(buf, "beef");

    PREFIX_(snprintf)(buf, sizeof(buf), "%X", 0xBEEFu);
    CHECK_STR_EQ(buf, "BEEF");
}

TEST_CASE("snprintf() pointer format")
{
    char buf[16];

    PREFIX_(snprintf)(buf, sizeof(buf), "%p", (void *)(uintptr_t)0xBEEF);
    CHECK_STR_EQ(buf, "0x0000BEEF");
}

TEST_CASE("snprintf() exact fit with no overflow")
{
    /* Buffer capacity exactly matches the formatted length. */
    char buf[6];
    int n = PREFIX_(snprintf)(buf, sizeof(buf), "%s", "Hello");

    CHECK(n == 5);
    CHECK_STR_EQ(buf, "Hello");
}

TEST_CASE("snprintf() truncates long string safely")
{
    char buf[4];

    PREFIX_(snprintf)(buf, sizeof(buf), "%s", "Hello, world!");

    CHECK(strlen(buf) < sizeof(buf));
    CHECK(buf[sizeof(buf) - 1] == '\0');
}

TEST_CASE("snprintf() negative fills buffer excactly")
{
    /*
     * Regression test: a sign character landing exactly on the last free
     * byte used to leave s == slen when falling into the shared `string:`
     * label, underflowing `slen - s - 1` and allowing an unbounded write
     * via memset()/_s_strncat(). Buffer sized so the sign character is the
     * last byte that can possibly fit before the width padding kicks in.
     */
    char buf[4];

    PREFIX_(snprintf)(buf, sizeof(buf), "%5d", -3);

    CHECK(strlen(buf) < sizeof(buf));
    CHECK(buf[sizeof(buf) - 1] == '\0');
}

TEST_CASE("snprintf() does not overflow already full buffer")
{
    char buf[4];

    PREFIX_(snprintf)(buf, sizeof(buf), "ABC%s", "more text");

    CHECK(strlen(buf) < sizeof(buf));
    CHECK(buf[sizeof(buf) - 1] == '\0');
    CHECK_MEM_EQ(buf, "ABC", 3);
}

TEST_CASE("snprintf() does not modify with zero length")
{
    char buf[1] = { 'X' };

    PREFIX_(snprintf)(buf, 0, "Hello");
    CHECK(buf[0] == 'X');
}
