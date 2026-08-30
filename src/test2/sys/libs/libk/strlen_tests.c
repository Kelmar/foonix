#include <string.h>

#include "tests.h"

TEST_CASE("strlen() basic functionality")
{
    CHECK(PREFIX_(strlen)("") == 0);
    CHECK(PREFIX_(strlen)("A") == 1);
    CHECK(PREFIX_(strlen)("Hello, world!") == 13);
}

TEST_CASE("strnlen() returns a value less than max")
{
    CHECK(PREFIX_(strnlen)("Hello", 100) == 5);
}

TEST_CASE("strnlen() caps at max length")
{
    /* No NUL within the first 3 bytes of "Hello" -> capped at max. */
    CHECK(PREFIX_(strnlen)("Hello", 3) == 3);
}

TEST_CASE("strnlen() returns zero with zero length")
{
    CHECK(PREFIX_(strnlen)("Hello", 0) == 0);
}

