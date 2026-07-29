#ifndef FOONIX_TESTS_TEST_IO_H
#define FOONIX_TESTS_TEST_IO_H

/*
 * Host-side output for the test binaries. Kept in its own header (no
 * <stdio.h> here) so files that need the project's own freestanding
 * <string.h>/<stdio.h> on their include path can still report results
 * without colliding with those minimal headers.
 *
 * extern "C" so C++ test files (test_kernel_args.cpp) link correctly
 * against the C-compiled implementation in test_io.c.
 */
#ifdef __cplusplus
extern "C" {
#endif

void test_io_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* FOONIX_TESTS_TEST_IO_H */
