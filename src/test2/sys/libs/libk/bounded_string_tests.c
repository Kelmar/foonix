#include <string.h>

#include "tests.h"

/*******************/

TEST_CASE("_s_strncat basic functionality")
{
    char buf[16] = "Hello, ";
    char *r = _s_strncat(buf, sizeof(buf), "world!", 6);

    CHECK(r == buf);
    CHECK_STR_EQ(buf, "Hello, world!");
}

TEST_CASE("_s_strncat truncates and terminates")
{
    char buf[10] = "Hello, "; /* 7 chars used, room for 2 more + NUL */
    char *r = _s_strncat(buf, sizeof(buf), "world!", 6);

    CHECK(r == buf);
    CHECK(strlen(buf) < sizeof(buf));
    CHECK_STR_EQ(buf, "Hello, wo");
}

TEST_CASE("_s_strncpy appends to empty string")
{
    char buf[16];
    memset(buf, 0, sizeof(buf));

    char *r = _s_strncpy(buf, sizeof(buf), "Hi", 2);

    CHECK(r == buf);
    CHECK_STR_EQ(buf, "Hi");
}

TEST_CASE("_s_strncat test propert termination when filling buffer.")
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

/*******************/

TEST_CASE("strncat basic functionality")
{
    char buf[16] = "Hello, ";
    char *r = PREFIX_(strncat)(buf, "world!", sizeof(buf));

    CHECK(r == buf);
    CHECK_STR_EQ(buf, "Hello, world!");
}

TEST_CASE("strncat truncates and terminates")
{
    char buf[10] = "Hello, ";
    char *r = PREFIX_(strncat)(buf, "world!", sizeof(buf));

    CHECK(r == buf);
    CHECK(strlen(buf) < sizeof(buf));
    CHECK(buf[sizeof(buf) - 1] == '\0');
}

TEST_CASE("strncat doesn't modify on overflow.")
{
    /* No NUL anywhere in buf's claimed `size` -- strncat can't safely
     * find an append point, so it must leave buf untouched rather than
     * scanning/writing past it. */
    char buf[4] = { 'A', 'B', 'C', 'D' };
    char *r = PREFIX_(strncat)(buf, "X", sizeof(buf));

    CHECK(r == buf);
    CHECK_MEM_EQ(buf, "ABCD", 4);
}

/*******************/

TEST_CASE("strncpy overwrites from start.")
{
    char buf[16] = "XXXXXXXXXXXXXXX";
    char *r = PREFIX_(strncpy)(buf, "Hi", sizeof(buf));

    CHECK(r == buf);
    CHECK_STR_EQ(buf, "Hi");
}

TEST_CASE("strncpy truncates without terminator.")
{
    char buf[4];
    memset(buf, 'X', sizeof(buf));

    char *r = PREFIX_(strncpy)(buf, "ABCDEFG", sizeof(buf));

    CHECK(r == buf);
    CHECK_MEM_EQ(buf, "ABCD", 4); /* no room for a NUL; strncpy just fills `size` */
}

TEST_CASE("strncpy pads remainder with nul characters.")
{
    /*
     * The standard contract (and the "per Linux man page" comment in
     * strncpy.c) requires ALL `size` bytes of dst to end up written --
     * either copied from src, or explicitly NUL-padded, with no gap.
     */
    char buf[8];
    memset(buf, 'X', sizeof(buf));

    PREFIX_(strncpy)(buf, "Hi", sizeof(buf));

    CHECK_MEM_EQ(buf, "Hi\0\0\0\0\0\0", 8);
}

/*******************/
