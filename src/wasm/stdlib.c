#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "js.h"

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

#define MEMORY_CHUNKS_CAPACITY 8192
#ifndef PROD
// #define ENABLE_MEMORY_DUMP
#endif

#ifdef ENABLE_MEMORY_DUMP
#include "../utils.h"
#endif

typedef struct Memory {
    size_t size;
    void *ptr;
    struct Memory *previous;
    struct Memory *next;
    bool used;
    bool allocated;
} Memory;

Memory memory_chunks[MEMORY_CHUNKS_CAPACITY] = {0};
Memory *base = NULL;

void exit(const int status) {
    _exit(status);
}

static Memory *get_memory_chunk(void) {
    for (size_t i = 0; i < MEMORY_CHUNKS_CAPACITY; ++i) {
        if (!memory_chunks[i].used) return &memory_chunks[i];
    }
    return NULL;
}

#ifdef ENABLE_MEMORY_DUMP
static void dump_memory_chunks(void) {
    assert(base != NULL);
    printf("=== Begin memory dump ===\n");
    for (Memory *memory_chunk = base; memory_chunk != NULL;
         memory_chunk = memory_chunk->next) {
        printf(
            "Memory(size=%lu, ptr=%p, previous=%p, next=%p, used=%s,"
            "allocated=%s)\n",
            memory_chunk->size, memory_chunk->ptr, memory_chunk->previous,
            memory_chunk->next, BOOL_TO_STR(memory_chunk->used),
            BOOL_TO_STR(memory_chunk->allocated));
    }
    printf("=== End memory dump ===\n");
}
#else
#define dump_memory_chunks()
#endif

void *malloc(const size_t size) {
    if (size == 0) return NULL;

    if (base == NULL) {
        base = memory_chunks;
        base->size = JS_get_memory_size();
        base->ptr = JS_get_heap_base();
        base->previous = NULL;
        base->next = NULL;
        base->used = true;
        base->allocated = false;
    }

    for (Memory *memory_chunk = base; memory_chunk != NULL;
         memory_chunk = memory_chunk->next) {
        assert(memory_chunk->used);
        if (memory_chunk->allocated) continue;
        if (memory_chunk->size == size) {
            memory_chunk->allocated = true;
            dump_memory_chunks();
            return memory_chunk->ptr;
        } else if (memory_chunk->size > size) {
            Memory *new_memory_chunk = get_memory_chunk();
            if (new_memory_chunk == NULL) {
                break;
            }
            new_memory_chunk->size = memory_chunk->size - size;
            new_memory_chunk->ptr = memory_chunk->ptr + size;
            new_memory_chunk->previous = memory_chunk;
            new_memory_chunk->next = memory_chunk->next;
            new_memory_chunk->used = true;
            new_memory_chunk->allocated = false;

            if (new_memory_chunk->next != NULL)
                new_memory_chunk->next->previous = new_memory_chunk;

            memory_chunk->size = size;
            memory_chunk->next = new_memory_chunk;
            memory_chunk->allocated = true;

            dump_memory_chunks();

            return memory_chunk->ptr;
        }
    }

    errno = ENOMEM;
    return NULL;
}

static Memory *get_memory_chunk_from_ptr(const void *const ptr) NONNULL();

static Memory *get_memory_chunk_from_ptr(const void *const ptr) {
    assert(base != NULL);
    for (Memory *memory_chunk = base; memory_chunk != NULL;
         memory_chunk = memory_chunk->next) {
        if (memory_chunk->ptr == ptr) return memory_chunk;
    }
    return NULL;
}

static void free_memory_chunk(Memory *memory_chunk) {
    if (memory_chunk == NULL) return;

    assert(memory_chunk->allocated);

    memory_chunk->allocated = false;

    Memory *const previous_memory_chunk = memory_chunk->previous;
    Memory *const next_memory_chunk = memory_chunk->next;

    if (previous_memory_chunk != NULL && !previous_memory_chunk->allocated) {
        previous_memory_chunk->size += memory_chunk->size;
        previous_memory_chunk->next = memory_chunk->next;
        memory_chunk->used = false;
        if (next_memory_chunk != NULL)
            next_memory_chunk->previous = previous_memory_chunk;
        memory_chunk = previous_memory_chunk;
    }

    if (next_memory_chunk != NULL && !next_memory_chunk->allocated) {
        memory_chunk->size += next_memory_chunk->size;
        memory_chunk->next = next_memory_chunk->next;
        if (next_memory_chunk->next != NULL)
            next_memory_chunk->next->previous = memory_chunk;
        next_memory_chunk->used = false;
    }
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) return malloc(size);

    assert(base != NULL);

    Memory *memory_chunk = get_memory_chunk_from_ptr(ptr);
    assert(memory_chunk != NULL);

    if (memory_chunk->size == size) return ptr;

    if (memory_chunk->size > size) {
        Memory *new_memory_chunk = get_memory_chunk();
        if (new_memory_chunk == NULL) {
            errno = ENOMEM;
            return NULL;
        }

        new_memory_chunk->size = memory_chunk->size - size;
        new_memory_chunk->previous = memory_chunk;
        new_memory_chunk->ptr = memory_chunk->ptr + size;
        new_memory_chunk->next = memory_chunk->next;
        if (new_memory_chunk->next != NULL)
            new_memory_chunk->next->previous = new_memory_chunk;
        new_memory_chunk->used = true;
        new_memory_chunk->allocated = false;

        memory_chunk->size = size;
        memory_chunk->next = new_memory_chunk;
        return ptr;
    }

    void *const new_ptr = malloc(size);
    if (new_ptr == NULL) return NULL;
    memcpy(new_ptr, ptr, memory_chunk->size);
    free_memory_chunk(memory_chunk);

    return new_ptr;
}

void free(void *const ptr) {
    assert(base != NULL);
    if (ptr == NULL) return;
    free_memory_chunk(get_memory_chunk_from_ptr(ptr));
    dump_memory_chunks();
}

char *getenv(const char *name) {
    (void)name;
    return NULL;
}

long __isoc23_strtol(const char *restrict ptr, const char **restrict endptr,
                     int base) NONNULL(1, 2);

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

long long __isoc23_strtoll(const char *restrict ptr,
                           const char **restrict endptr, int base)
    NONNULL(1, 2);

long long __isoc23_strtoll(const char *restrict ptr,
                           const char **restrict endptr, int base) {
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
