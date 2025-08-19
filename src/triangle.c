#include "triangle.h"

#include <assert.h>
#include <stdlib.h>

#include "utils.h"

Triangle3D *triangle3D_init_v3f(
    const v3f *const restrict v1, const v3f *const restrict v2,
    const v3f *const restrict v3, const v2f uv1, const v2f uv2, const v2f uv3,
    const uint8_t edges, const Texture *const texture, const Color color) {
    const v4f v1_v4f = v3f_to_v4f(*v1);
    const v4f v2_v4f = v3f_to_v4f(*v2);
    const v4f v3_v4f = v3f_to_v4f(*v3);
    return triangle3D_init_v4f(&v1_v4f, &v2_v4f, &v3_v4f, uv1, uv2, uv3, edges,
                               texture, color);
}

Triangle3D *triangle3D_init_v4f(
    const v4f *const restrict v1, const v4f *const restrict v2,
    const v4f *const restrict v3, const v2f uv1, const v2f uv2, const v2f uv3,
    const uint8_t edges, const Texture *const texture, const Color color) {
    assert(v1 != NULL);
    assert(v2 != NULL);
    assert(v3 != NULL);

    Triangle3D *const self =
        malloc_or_exit(sizeof(*self), "failed to create triangle 3D");

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

void triangle3D_destroy(Triangle3D *const self) {
    assert(self != NULL);
    free(self);
}

v3f triangle3D_get_normal(const Triangle3D *const self) {
    assert(self != NULL);
    return v3f_cross_product(v3f_sub(self->v2.xyz, self->v1.xyz),
                             v3f_sub(self->v3.xyz, self->v1.xyz));
}
