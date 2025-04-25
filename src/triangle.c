#include "triangle.h"

#include <assert.h>
#include <stdlib.h>

#include "utils.h"

Triangle3D *triangle3D_init(const v3f *const restrict v1,
                            const v3f *const restrict v2,
                            const v3f *const restrict v3, const uint8_t edges,
                            const Color color) {
    assert(v1 != NULL);
    assert(v2 != NULL);
    assert(v3 != NULL);

    Triangle3D *const self =
        malloc_or_exit(sizeof(*self), "failed to create triangle 3D");

    self->v1 = *v1;
    self->v2 = *v2;
    self->v3 = *v3;
    self->edges = edges;
    self->color = color;

    return self;
}

void triangle3D_destroy(Triangle3D *const self) {
    assert(self != NULL);
    free(self);
}

v3f triangle3D_get_normal(const Triangle3D *const self) {
    assert(self != NULL);
    return v3f_cross_product(v3f_sub(self->v2, self->v1),
                             v3f_sub(self->v3, self->v1));
}
