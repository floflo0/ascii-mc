#include "utils.h"

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"

#ifndef PROD
#define location file, line, function_name,
#else
#define location
#endif

void *_malloc_or_exit(location_param const size_t size,
                      const char *restrict error_message_format, ...) {
    assert(error_message_format != NULL);
    void *const pointer = malloc(size);
    if (pointer == NULL) {
        va_list args;
        va_start(args, error_message_format);
        _log_errorf_errno(location error_message_format, args);
        log_debugf("trying to allocate %lu bytes", size);
        exit(EXIT_FAILURE);
    }

    return pointer;
}

void *_realloc_or_exit(location_param void *restrict pointer, const size_t size,
                       const char *restrict error_message_format, ...) {
    assert(size > 0);
    assert(error_message_format != NULL);
    void *const new_pointer = realloc(pointer, size);
    if (new_pointer == NULL) {
        va_list args;
        va_start(args, error_message_format);
        _log_errorf_errno(location error_message_format, args);
        exit(EXIT_FAILURE);
    }

    return new_pointer;
}

uint64_t get_time_miliseconds(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

uint64_t get_time_microseconds(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return now.tv_sec * 1000000 + now.tv_nsec / 1000;
}

void copy_string(char *const restrict string_destination,
                 const char *const restrict string_source,
                 const size_t max_size) {
    assert(string_destination != NULL);
    assert(string_source != NULL);
    assert(max_size > 0);
    size_t i;
    const size_t limit = max_size - 1;
    for (i = 0; i < limit && string_source[i] != '\0'; ++i) {
        string_destination[i] = string_source[i];
    }
    string_destination[i] = '\0';
}

bool parse_int(const char *const restrict string, int *const restrict value) {
    assert(string != NULL);
    assert(value != NULL);
    char *end_pointer;
    const long value_long = strtol(string, &end_pointer, 10);
    assert(end_pointer != NULL);
    if (*end_pointer != '\0' || value_long > INT_MAX || value_long < INT_MIN) {
        return false;
    }
    *value = value_long;
    return true;
}

bool parse_uint32(const char *const restrict string,
                  uint32_t *const restrict value) {
    assert(string != NULL);
    assert(value != NULL);
    char *end_pointer;
    const long value_long = strtol(string, &end_pointer, 10);
    assert(end_pointer != NULL);
    if (*end_pointer != '\0' || value_long > UINT32_MAX || value_long < 0) {
        return false;
    }
    *value = value_long;
    return true;
}
