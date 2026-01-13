#pragma once

#include <assert.h>

#include "array_defs.h"
#include "utils.h"

void array_destroy(const Array *const self);

#define DEFINE_ARRAY(name, Name, type)                           \
    [[gnu::nonnull(1)]]                                          \
    void name##_array_init(Name##Array *const self,              \
                           const size_t default_capacity);       \
    [[gnu::nonnull]]                                             \
    size_t name##_array_grow(Name##Array *const self);           \
    [[gnu::nonnull(1)]]                                          \
    void name##_array_push(Name##Array *const self, type value); \
    [[gnu::nonnull(1)]]                                          \
    void name##_array_remove(Name##Array *const self, const size_t index);

#define ARRAY_IMPLEMENTATION(name, Name, type)                                \
    void name##_array_init(Name##Array *const self,                           \
                           const size_t default_capacity) {                   \
        assert(self != NULL);                                                 \
        assert(0 < default_capacity);                                         \
        self->array = malloc_or_exit(sizeof(*self->array) * default_capacity, \
                                     "failed to create a " #Name "Array");    \
        self->length = 0;                                                     \
        self->capacity = default_capacity;                                    \
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
