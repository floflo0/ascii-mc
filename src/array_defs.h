#pragma once

#include <stddef.h>

typedef struct {
    void *array;
    size_t length;
    size_t capacity;
} Array;

#define DEFINE_ARRAY_TYPE(Name, type) \
    typedef struct {                  \
        type *array;                  \
        size_t length;                \
        size_t capacity;              \
    } Name##Array;
