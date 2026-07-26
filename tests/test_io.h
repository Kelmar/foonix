#ifndef FOONIX_TESTS_TEST_IO_H
#define FOONIX_TESTS_TEST_IO_H

/*
 * Host-side output for the test binary. Kept in its own header (no
 * <stdio.h> here) so files that need the project's own freestanding
 * <string.h>/<stdio.h> on their include path can still report results
 * without colliding with those minimal headers.
 */
void test_io_printf(const char *fmt, ...);

#endif /* FOONIX_TESTS_TEST_IO_H */
