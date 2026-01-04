#include "triangle.h"

#include <assert.h>
#include <stdlib.h>

#include "utils.h"

Triangle3D *triangle3D_init_v3f(
    const v3f *const restrict v1, const v3f *const restrict v2,
    const v3f *const restrict v3, const v2f uv1, const v2f uv2, const v2f uv3,
    const uint8_t edges, const Texture *const restrict texture,
    const Color color, Triangle3DArena *const restrict arena) {
    assert(v1 != NULL);
    assert(v2 != NULL);
    assert(v3 != NULL);
    assert(arena != NULL);
    const v4f v1_v4f = v3f_to_v4f(*v1);
    const v4f v2_v4f = v3f_to_v4f(*v2);
    const v4f v3_v4f = v3f_to_v4f(*v3);
    return triangle3D_init_v4f(&v1_v4f, &v2_v4f, &v3_v4f, uv1, uv2, uv3, edges,
                               texture, color, arena);
}

[[gnu::nonnull]] [[gnu::returns_nonnull]]
static inline Triangle3D *triange3D_arena_alloc(Triangle3DArena *const self) {
    assert(self != NULL);
    assert(self->next < self->end);

    Triangle3D *const next = self->next++;
    if (self->next == self->end) {
        const size_t capacity = self->end - self->start;
        Triangle3DArena *const next_arena = malloc_or_exit(
            sizeof(*next_arena) + capacity * sizeof(*next_arena->start),
            "failed to grow triangle 3D arena");
        self->start = (void *)next_arena + sizeof(*next_arena);
        self->end = self->start + capacity;
        self->next = self->start;
        next_arena->next_arena = self->next_arena;
        self->next_arena = next_arena;
    }

    return next;
    ;
}

Triangle3D *triangle3D_init_v4f(
    const v4f *const restrict v1, const v4f *const restrict v2,
    const v4f *const restrict v3, const v2f uv1, const v2f uv2, const v2f uv3,
    const uint8_t edges, const Texture *const restrict texture,
    const Color color, Triangle3DArena *const restrict arena) {
    assert(v1 != NULL);
    assert(v2 != NULL);
    assert(v3 != NULL);
    assert(arena != NULL);

    Triangle3D *const self = triange3D_arena_alloc(arena);

    self->v1 = *v1;
    self->v2 = *v2;
    self->v3 = *v3;
    self->uv1 = uv1;
    self->uv2 = uv2;
    self->uv3 = uv3;
    self->edges = edges;
    self->texture = texture;
    self->color = color;

    return self;
}

v3f triangle3D_get_normal(const Triangle3D *const self) {
    assert(self != NULL);
    return v3f_cross_product(v3f_sub(self->v2.xyz, self->v1.xyz),
                             v3f_sub(self->v3.xyz, self->v1.xyz));
}

Triangle3DArena *triangle3D_arena_create(const size_t capacity) {
    assert(capacity > 0);

    Triangle3DArena *const self =
        malloc_or_exit(sizeof(*self) + capacity * sizeof(*self->start),
                       "failed to create triangle 3D arena");
    self->start = (void *)self + sizeof(*self);
    self->end = self->start + capacity;
    self->next = self->start;
    self->next_arena = NULL;

    return self;
}

void triangle3D_arena_destroy(Triangle3DArena *const self) {
    assert(self != NULL);
    if (self->next_arena != NULL) {
        triangle3D_arena_destroy(self->next_arena);
    }
    free(self);
}
