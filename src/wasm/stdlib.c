#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

void exit(const int status) {
    _exit(status);
}

char *getenv([[gnu::unused]] const char *name) {
    return NULL;
}

[[gnu::nonnull(1, 2)]]
long __isoc23_strtol(const char *restrict ptr, const char **restrict endptr,
                     int base);

long __isoc23_strtol(const char *restrict ptr, const char **restrict endptr,
                     int base) {
    assert(ptr != NULL);
    assert(endptr != NULL);

    if (base != 10) {
        errno = EINVAL;
        *endptr = NULL;
        return 0;
    }

    while (isspace(*ptr)) ++ptr;

    const bool is_negative = *ptr == '-';
    if (*ptr == '+') ++ptr;

    long parsed_value = 0;
    for (; isdigit(*ptr); ++ptr) {
        const long previous_parsed_value = parsed_value;
        parsed_value = parsed_value * 10 - *ptr + '0';
        if (parsed_value > previous_parsed_value) {
            errno = ERANGE;
            parsed_value = is_negative ? LONG_MIN : LONG_MAX;
            goto return_;
        }
    }

    if (!is_negative) {
        if (parsed_value == LONG_MIN) {
            errno = ERANGE;
            parsed_value = LONG_MAX;
            goto return_;
        }
        parsed_value = -parsed_value;
    }

return_:
    *endptr = ptr;
    return parsed_value;
}

[[gnu::nonnull(1, 2)]]
long long __isoc23_strtoll(const char *restrict ptr,
                           const char **restrict endptr, const int base);

long long __isoc23_strtoll(const char *restrict ptr,
                           const char **restrict endptr, const int base) {
    assert(ptr != NULL);
    assert(endptr != NULL);
    if (base != 10) {
        errno = EINVAL;
        *endptr = NULL;
        return 0;
    }

    while (isspace(*ptr)) ++ptr;

    const bool is_negative = *ptr == '-';
    if (*ptr == '+') ++ptr;

    long long parsed_value = 0;
    for (; isdigit(*ptr); ++ptr) {
        const long previous_parsed_value = parsed_value;
        parsed_value = parsed_value * 10 - *ptr + '0';
        if (parsed_value > previous_parsed_value) {
            errno = ERANGE;
            parsed_value = is_negative ? LLONG_MIN : LLONG_MAX;
            goto return_;
        }
    }

    if (!is_negative) {
        if (parsed_value == LLONG_MIN) {
            errno = ERANGE;
            parsed_value = LLONG_MAX;
            goto return_;
        }
        parsed_value = -parsed_value;
    }

return_:
    *endptr = ptr;
    return parsed_value;
}
