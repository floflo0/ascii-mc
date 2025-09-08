#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#ifndef NDEBUG
void __assert_fail(const char *assertion, const char *file, unsigned int line,
                   const char *function) {
    fprintf(stderr, "%s:%u: %s: Assertion `%s' failed.\n", file, line, function,
            assertion);
    _exit(134);
}
#endif
