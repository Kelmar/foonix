#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#include "../include/string.h"

// Time for each test in seconds.
#define TEST_TIME 10

#define BUF_SIZE 1000000

typedef void (*test_t)(void);

void run_test(const char *name, test_t test)
{
    time_t start = time(NULL);
    int cnt = 0;
    time_t now;
    int secs;

    printf("Testing %s: ", name);
    fflush(stdout);

    for (;;)
    {
        now = time(NULL);

        secs = now - start;

        if (secs >= TEST_TIME)
            break;

        (test)();
        ++cnt;
    }

    printf("%d runs in %d seconds.\n", cnt, secs);
    fflush(stdout);
}

void cmemset(void *dst, unsigned char val, size_t sz)
{
    size_t i;

    for (i = 0; i < sz; ++i)
        ((char *)dst)[i] = val;
}

void test1(void)
{
    char buf[BUF_SIZE];

    _foo_memset(buf, 0, sizeof(buf));

#if 0
    /* Validate that it worked.... */
    for (size_t i = 0; i < sizeof(buf); ++i)
    {
        if (buf[i] != 0)
        {
            printf("Test failed at byte: %d!\r\n", i);
            printf("BYTE: 0x%02X\r\n", buf[i]);
            exit(-1);
        }
    }
#endif
}

void test2(void)
{
    char buf[BUF_SIZE];

    cmemset(buf, 0, sizeof(buf));
}

void test3()
{
    char buf[BUF_SIZE];

    memset(buf, 0, sizeof(buf));
}

int main(void)
{
    run_test("FooNIX memset()", test1);
    run_test("cmemset()", test2);
    run_test("stdc memset()", test3);

    return 0;
}
