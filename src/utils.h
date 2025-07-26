#pragma once

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define RETURNS_NONNULL __attribute__((returns_nonnull))

#define RAD(angle) (2.0f * M_PI * angle / 360.0f)
#define DEG(angle) (360.0f * angle / (2.0f * M_PI))

#define POSITIVE_MOD(a, b) (((a) % (b) + (b)) % (b))

#ifndef PROD
#define file_and_line_param const char *restrict file, const size_t line,
#define malloc_or_exit(size, error_message_format, ...)       \
    _malloc_or_exit(__FILE__, __LINE__, __FUNCTION__, (size), \
                    (error_message_format)__VA_OPT__(, ) __VA_ARGS__)
#define realloc_or_exit(pointer, size, error_message_format, ...)         \
    _realloc_or_exit(__FILE__, __LINE__, __FUNCTION__, (pointer), (size), \
                     (error_message_format)__VA_OPT__(, ) __VA_ARGS__)

void *_malloc_or_exit(const char *const restrict file, const size_t line,
                      const char *const restrict function_name,
                      const size_t size,
                      const char *const restrict error_message_format, ...)
    NONNULL(1, 3, 5) RETURNS_NONNULL;

void *_realloc_or_exit(const char *const restrict file, const size_t line,
                       const char *const restrict function_name,
                       void *restrict pointer, const size_t size,
                       const char *const restrict error_message_format, ...)
    NONNULL(1, 3, 4, 6) RETURNS_NONNULL;
#else
#define file_and_line_param
#define malloc_or_exit(size, error_message) \
    _malloc_or_exit((size), (error_message))
#define realloc_or_exit(pointer, size, error_message) \
    _realloc_or_exit((pointer), (size), (error_message))

void *_malloc_or_exit(const size_t size, const char *error_message_format, ...)
    NONNULL(2) RETURNS_NONNULL;

void *_realloc_or_exit(void *pointer, const size_t size,
                       const char *error_message_format, ...)
    NONNULL(3) RETURNS_NONNULL;
#endif

uint64_t get_time_miliseconds(void);
uint64_t get_time_microseconds(void);

void copy_string(char *const restrict string_destination,
                 const char *const restrict string_source,
                 const size_t max_size) NONNULL(1, 2);

bool parse_int(const char *const restrict string, int *const restrict value)
    NONNULL();

bool parse_uint32(const char *const restrict string,
                  uint32_t *const restrict value) NONNULL();

static inline int min_int(const int a, const int b) {
    return a < b ? a : b;
}

static inline int min3_int(const int a, const int b, const int c) {
    return min_int(a, min_int(b, c));
}

static inline int max_int(const int a, const int b) {
    return a > b ? a : b;
}

static inline int max3_int(const int a, const int b, const int c) {
    return max_int(a, max_int(b, c));
}

static inline float min3_float(const float a, const float b, const float c) {
    return fminf(a, fminf(b, c));
}

static inline float clamp_float(const float value, const float minimum,
                                const float maximum) {
    return fminf(maximum, fmaxf(minimum, value));
}
