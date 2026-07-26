#include "test_io.h"
#include "tests.h"

extern int g_checks;
extern int g_failures;

int main(void)
{
    run_memory_tests();
    run_strlen_tests();
    run_bounded_string_tests();
    run_vsnprintf_tests();

    test_io_printf("%d/%d checks passed\n", g_checks - g_failures, g_checks);

    return g_failures ? 1 : 0;
}
