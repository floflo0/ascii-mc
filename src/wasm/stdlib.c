#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "js.h"
#include "wasm.h"

#define MEMORY_CHUNKS_CAPACITY 8192

typedef struct MemoryChunk {
    size_t size;
    void *ptr;
    struct MemoryChunk *previous;
    struct MemoryChunk *next;
    struct MemoryChunk *next_free;
    struct MemoryChunk *previous_free;
    bool used;
    bool allocated;
} MemoryChunk;

extern unsigned char __heap_base;

static MemoryChunk memory_chunks[MEMORY_CHUNKS_CAPACITY] = {0};
static MemoryChunk *next_free_memory_chunk = NULL;
static MemoryChunk *next_unused_memory_chunk = NULL;

static MemoryChunk *get_next_unused_memory_chunk(void) {
    if (next_unused_memory_chunk == NULL) return NULL;

    MemoryChunk *const memory_chunk = next_unused_memory_chunk;
    next_unused_memory_chunk = next_unused_memory_chunk->next;
    return memory_chunk;
}

static inline size_t align_size(const size_t size) {
    return (size + (sizeof(max_align_t) - 1)) & ~(sizeof(max_align_t) - 1);
}

void malloc_init(void) {
    assert(next_free_memory_chunk == NULL);
    next_free_memory_chunk = memory_chunks;
    assert(next_free_memory_chunk->previous == NULL);
    assert(next_free_memory_chunk->next == NULL);
    assert(next_free_memory_chunk->next_free == NULL);
    assert(next_free_memory_chunk->previous_free == NULL);
    assert(!next_free_memory_chunk->used);
    assert(!next_free_memory_chunk->allocated);
    next_free_memory_chunk->size = JS_get_memory_size();
    next_free_memory_chunk->ptr = &__heap_base;
    next_free_memory_chunk->used = true;

    next_unused_memory_chunk = &memory_chunks[1];
    for (size_t i = 1; i < MEMORY_CHUNKS_CAPACITY - 1; ++i) {
        memory_chunks[i].next = &memory_chunks[i + 1];
    }
}

[[gnu::nonnull]]
static void memory_chunk_add_to_free_list(MemoryChunk *const self) {
    assert(self != NULL);
    assert(!self->allocated);
    self->next_free = next_free_memory_chunk;
    if (next_free_memory_chunk != NULL) {
        next_free_memory_chunk->previous_free = self;
    }
    next_free_memory_chunk = self;
}

[[gnu::nonnull(1)]]
static bool memory_chunk_split(MemoryChunk *const self, const size_t new_size) {
    assert(self != NULL);
    assert(self->used);
    assert(new_size > 0);
    assert(new_size % sizeof(max_align_t) == 0);
    MemoryChunk *const new_memory_chunk = get_next_unused_memory_chunk();
    if (new_memory_chunk == NULL) return false;

    new_memory_chunk->size = self->size - new_size;
    new_memory_chunk->ptr = self->ptr + new_size;
    new_memory_chunk->previous = self;
    new_memory_chunk->next = self->next;
    if (new_memory_chunk->next != NULL) {
        new_memory_chunk->next->previous = new_memory_chunk;
    }
    new_memory_chunk->used = true;
    new_memory_chunk->allocated = false;
    self->size = new_size;
    self->next = new_memory_chunk;
    memory_chunk_add_to_free_list(new_memory_chunk);

    return true;
}

[[gnu::nonnull]]
static void memory_chunk_remove_from_free_list(MemoryChunk *const self) {
    assert(self != NULL);
    assert(self->used);
    if (self == next_free_memory_chunk) {
        next_free_memory_chunk = next_free_memory_chunk->next_free;
        if (next_free_memory_chunk != NULL) {
            next_free_memory_chunk->previous_free = NULL;
        }
    } else {
        assert(self->previous_free != NULL);
        MemoryChunk *const next_free = self->next_free;
        MemoryChunk *const previous_free = self->previous_free;
        previous_free->next_free = next_free;
        if (next_free != NULL) {
            next_free->previous_free = previous_free;
        }
    }
}

[[gnu::nonnull]]
static void memory_chunk_allocate(MemoryChunk *const self) {
    assert(self != NULL);
    assert(self->used);
    assert(!self->allocated);
    self->allocated = true;
    *(MemoryChunk **)self->ptr = self;
    memory_chunk_remove_from_free_list(self);
}

void *malloc(size_t size) {
    size += sizeof(MemoryChunk *);
    size = align_size(size);

    for (MemoryChunk *memory_chunk = next_free_memory_chunk;
         memory_chunk != NULL; memory_chunk = memory_chunk->next_free) {
        assert(memory_chunk->used);

        if (memory_chunk->allocated || memory_chunk->size < size) continue;

        if (memory_chunk->size > size) {
            if (!memory_chunk_split(memory_chunk, size)) {
                // We can't use a new memory chunk but we can still find a free
                // one that match the requested size.
                continue;
            }
        }

        memory_chunk_allocate(memory_chunk);
        return memory_chunk->ptr + sizeof(MemoryChunk *);
    }

    errno = ENOMEM;
    return NULL;
}

[[gnu::nonnull]] [[gnu::returns_nonnull]]
static inline MemoryChunk *get_memory_chunk_from_ptr(void *const ptr) {
    assert(ptr != NULL);
    MemoryChunk *const memory_chunk = *((MemoryChunk **)ptr - 1);
    assert(memory_chunk != NULL);
    assert(memory_chunk->used);
    assert(memory_chunk->allocated);
    return memory_chunk;
}

[[gnu::nonnull]]
static void memory_chunk_make_unused(MemoryChunk *const self) {
    assert(self != NULL);
    assert(self->used);
    self->used = false;
    self->next = next_unused_memory_chunk;
    next_unused_memory_chunk = self;
}

[[gnu::nonnull]]
static void memory_chunk_free(MemoryChunk *self) {
    assert(self != NULL);
    assert(self->allocated);

    self->allocated = false;
    bool add_to_free_list = true;

    MemoryChunk *const previous_memory_chunk = self->previous;
    MemoryChunk *const next_memory_chunk = self->next;

    if (previous_memory_chunk != NULL && !previous_memory_chunk->allocated) {
        previous_memory_chunk->size += self->size;
        previous_memory_chunk->next = self->next;
        if (next_memory_chunk != NULL) {
            next_memory_chunk->previous = previous_memory_chunk;
        }
        memory_chunk_make_unused(self);
        self = previous_memory_chunk;
        add_to_free_list = false;
    }

    if (next_memory_chunk != NULL && !next_memory_chunk->allocated) {
        self->size += next_memory_chunk->size;
        self->next = next_memory_chunk->next;
        if (next_memory_chunk->next != NULL) {
            next_memory_chunk->next->previous = self;
        }
        memory_chunk_remove_from_free_list(next_memory_chunk);
        memory_chunk_make_unused(next_memory_chunk);
    }

    if (add_to_free_list) {
        memory_chunk_add_to_free_list(self);
    }
}

void *realloc(void *const ptr, size_t requested_size) {
    if (ptr == NULL) return malloc(requested_size);

    const size_t size = align_size(requested_size + sizeof(MemoryChunk *));

    MemoryChunk *const memory_chunk = get_memory_chunk_from_ptr(ptr);
    assert(memory_chunk != NULL);

    const size_t memory_chunk_size = memory_chunk->size;

    if (memory_chunk_size == size) return ptr;

    if (memory_chunk_size > size) {
        if (!memory_chunk_split(memory_chunk, size)) {
            errno = ENOMEM;
            return NULL;
        }
        MemoryChunk *const next_memory_chunk = memory_chunk->next;
        assert(memory_chunk->next != NULL);
        MemoryChunk *const next_next_memory_chunk = next_memory_chunk->next;
        if (next_next_memory_chunk && !next_next_memory_chunk->allocated) {
            next_memory_chunk->size += next_next_memory_chunk->size;
            next_memory_chunk->next = next_next_memory_chunk->next;
            if (next_memory_chunk->next) {
                next_memory_chunk->next->previous = next_memory_chunk;
            }
            memory_chunk_remove_from_free_list(next_next_memory_chunk);
            memory_chunk_make_unused(next_next_memory_chunk);
        }
        return ptr;
    }

    MemoryChunk *const next_memory_chunk = memory_chunk->next;
    if (next_memory_chunk && !next_memory_chunk->allocated) {
        if (memory_chunk_size + next_memory_chunk->size < size) {
            goto alloc_new_chunk;
        }
        memory_chunk->size = size;
        if (memory_chunk_size + next_memory_chunk->size == size) {
            memory_chunk->next = next_memory_chunk->next;
            if (next_memory_chunk->next) {
                next_memory_chunk->next->previous = memory_chunk;
            }
            memory_chunk_remove_from_free_list(next_memory_chunk);
            memory_chunk_make_unused(next_memory_chunk);
        } else {
            const size_t diff = size - memory_chunk_size;
            next_memory_chunk->size -= diff;
            next_memory_chunk->ptr += diff;
        }
        return ptr;
    }

alloc_new_chunk:
    void *const new_ptr = malloc(requested_size);
    if (new_ptr == NULL) return NULL;
    memcpy(new_ptr, ptr, memory_chunk_size - sizeof(MemoryChunk *));
    memory_chunk_free(memory_chunk);

    return new_ptr;
}

void free(void *const ptr) {
    assert(next_free_memory_chunk != NULL);
    if (ptr == NULL) return;
    MemoryChunk *const memory_chunk = get_memory_chunk_from_ptr(ptr);
    memory_chunk_free(memory_chunk);
}

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
