#include <string.h>

#include "tests.h"

TEST_CASE("Basic memcpy() functionality")
{
    char dst[8] = { 0 };
    const char src[] = "ABCDEFG";

    PREFIX_(memcpy)(dst, src, 7);
    CHECK_MEM_EQ(dst, "ABCDEFG", 7);
}

TEST_CASE("memcpy() does not modify on zero length")
{
    char dst[4] = { 'X', 'X', 'X', 'X' };
    const char src[4] = { 'Y', 'Y', 'Y', 'Y' };

    PREFIX_(memcpy)(dst, src, 0);
    CHECK_MEM_EQ(dst, "XXXX", 4);
}

TEST_CASE("memmove does not overlap")
{
    char buf[8] = "ABCDEFG";
    char dst[8] = { 0 };

    PREFIX_(memmove)(dst, buf, 7);
    CHECK_MEM_EQ(dst, "ABCDEFG", 7);
}

TEST_CASE("memmove handles overlapping case correctly")
{
    /* dst < src: shifting "BCDEFG" one position left. */
    char buf[8] = "ABCDEFG";

    PREFIX_(memmove)(buf, buf + 1, 6);
    CHECK_MEM_EQ(buf, "BCDEFGG", 7);
}

TEST_CASE("memmove handles backwards overlaps correctly")
{
    char buf[8] = "ABCDEF\0";

    PREFIX_(memmove)(buf + 1, buf, 6);
    CHECK_MEM_EQ(buf, "AABCDEF", 7);
}

TEST_CASE("memmove handles same pointer case")
{
    char buf[] = "ABCDEF";

    PREFIX_(memmove)(buf, buf, 6);
    CHECK_MEM_EQ(buf, "ABCDEF", 6);
}

TEST_CASE("memmove with zero length does not modify")
{
    char buf[4] = { 'A', 'B', 'C', 'D' };

    PREFIX_(memmove)(buf + 2, buf, 0);
    CHECK_MEM_EQ(buf, "ABCD", 4);
}

TEST_CASE("memset fills as expected")
{
    char buf[6];

    PREFIX_(memset)(buf, 'Z', sizeof(buf));
    CHECK_MEM_EQ(buf, "ZZZZZZ", 6);
}

TEST_CASE("memset does not modify with zero length")
{
    char buf[4] = { 'A', 'B', 'C', 'D' };

    PREFIX_(memset)(buf, 'Z', 0);
    CHECK_MEM_EQ(buf, "ABCD", 4);
}

TEST_CASE("memchr returns correct pointer result")
{
    const char buf[] = "ABCDEF";

    CHECK(PREFIX_(memchr)(buf, 'D', sizeof(buf)) == buf + 3);
}

TEST_CASE("memchr returns null on not found case")
{
    const char buf[] = "ABCDEF";

    CHECK(PREFIX_(memchr)(buf, 'Z', sizeof(buf) - 1) == NULL);
}

TEST_CASE("memchr finds embedded null character")
{
    const char buf[] = "AB";

    CHECK(PREFIX_(memchr)(buf, '\0', sizeof(buf)) == buf + 2);
}

TEST_CASE("memcmp returns equal on identical strings")
{
    CHECK(PREFIX_(memcmp)("ABCDEF", "ABCDEF", 6) == 0);
}

TEST_CASE("memcmp orders by the first difference found")
{
    CHECK(PREFIX_(memcmp)("ABC", "ABD", 3) < 0);
    CHECK(PREFIX_(memcmp)("ABD", "ABC", 3) > 0);

    /* A later byte must not override an earlier decisive difference. */
    CHECK(PREFIX_(memcmp)("BA", "AZ", 2) > 0);
}
