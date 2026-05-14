#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef NDEBUG
void __assert_fail(const char *assertion, const char *file, unsigned int line,
                   const char *function) {
    fprintf(stderr, "%s:%u: %s: Assertion `%s' failed.\n", file, line, function,
            assertion);
    abort();
}
#endif
