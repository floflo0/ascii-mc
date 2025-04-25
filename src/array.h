#pragma once

#include <assert.h>
#include <stdlib.h>

#include "utils.h"

typedef struct {
    void *array;
    size_t length;
    size_t capacity;
} Array;

void array_destroy(Array *const self);

#define DEFINE_ARRAY(name, Name, type)                                      \
    typedef struct {                                                        \
        type *array;                                                        \
        size_t length;                                                      \
        size_t capacity;                                                    \
    } Name##Array;                                                          \
    Name##Array *name##_array_create(const size_t default_capacity)         \
        RETURNS_NONNULL;                                                    \
    size_t name##_array_grow(Name##Array *const self) NONNULL();            \
    void name##_array_push(Name##Array *const self, type value) NONNULL(1); \
    void name##_array_remove(Name##Array *const self, const size_t index)   \
        NONNULL(1);

#define ARRAY_IMPLEMENTATION(name, Name, type)                                \
    Name##Array *name##_array_create(const size_t default_capacity) {         \
        assert(0 < default_capacity);                                         \
        Name##Array *const self = malloc_or_exit(                             \
            sizeof(*self), "failed to create a " #Name "Array");              \
        self->array = malloc_or_exit(sizeof(*self->array) * default_capacity, \
                                     "failed to create a " #Name "Array");    \
        self->length = 0;                                                     \
        self->capacity = default_capacity;                                    \
        return self;                                                          \
    }                                                                         \
                                                                              \
    size_t name##_array_grow(Name##Array *const self) {                       \
        assert(self != NULL);                                                 \
        assert(self->array != NULL);                                          \
        if (self->length == self->capacity) {                                 \
            self->capacity *= 2;                                              \
            self->array = realloc_or_exit(                                    \
                self->array, sizeof(*self->array) * self->capacity,           \
                "failed to resize array");                                    \
        }                                                                     \
        return self->length++;                                                \
    }                                                                         \
                                                                              \
    void name##_array_push(Name##Array *const self, type value) {             \
        assert(self != NULL);                                                 \
        assert(self->array != NULL);                                          \
        const size_t i = name##_array_grow(self);                             \
        self->array[i] = value;                                               \
    }                                                                         \
                                                                              \
    void name##_array_remove(Name##Array *const self, const size_t index) {   \
        assert(self != NULL);                                                 \
        assert(self->array != NULL);                                          \
        const size_t end = --self->length;                                    \
        for (size_t i = index; i < end; ++i) {                                \
            self->array[i] = self->array[i + 1];                              \
        }                                                                     \
    }
