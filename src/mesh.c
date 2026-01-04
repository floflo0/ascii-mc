#include "mesh.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#include "array.h"
#include "camera.h"
#include "config.h"
#include "triangle3D_array.h"
#include "window.h"

typedef struct {
    v3f normal;
    float d;
} Plane;

Mesh *mesh_create(const size_t preallocate_vertices_size,
                  const size_t preallocate_triangles_size) {
    Mesh *const self = malloc_or_exit(sizeof(*self), "failed to create mesh");

    self->vertices = v3f_array_create(preallocate_vertices_size);
    self->triangles = triangle_index_array_create(preallocate_triangles_size);

    return self;
}

void mesh_destroy(Mesh *const self) {
    assert(self != NULL);
    assert(self->vertices != NULL);
    assert(self->triangles != NULL);
    array_destroy((Array *)self->vertices);
    array_destroy((Array *)self->triangles);
    free(self);
}

[[gnu::nonnull]] [[gnu::returns_nonnull]]
static inline v3f *mesh_get_viewed_vertices(
    const Mesh *const restrict self, const Camera *const restrict camera) {
    assert(self != NULL);
    assert(self->vertices != NULL);
    assert(camera != NULL);

    m4f view_matrix;
    camera_get_view_matrix(camera, view_matrix);

    v3f *const view_vertices =
        malloc_or_exit(sizeof(*view_vertices) * self->vertices->length,
                       "failed to get viewed vertices");
    for (size_t i = 0; i < self->vertices->length; ++i) {
        view_vertices[i] =
            mul_m4f_v3f(view_matrix, self->vertices->array[i]).xyz;
    }
    return view_vertices;
}

[[gnu::nonnull(1)]]
static v4f plane_intersect(const Plane *const plane, const v4f v1,
                           const v4f v2) {
    assert(plane != NULL);

    const v3f direction = v4f_sub(v1, v2).xyz;

    const float numerator = plane->d - v3f_dot(plane->normal, v1.xyz);
    const float denominator = v3f_dot(plane->normal, direction);

    assert(denominator != 0.0f);

    return v4f_add_v3f(v1, v3f_mul(direction, numerator / denominator));
}

static float get_interpolation(const v3f v1, const v3f v2, const v3f v3) {
    const v3f v3_sub_v1 = v3f_sub(v3, v1);
    return v3f_dot(v3f_sub(v2, v1), v3_sub_v1) / v3f_norm_squared(v3_sub_v1);
}

[[gnu::nonnull(1, 3, 4, 5)]]
static void planes_clip_triangle(const Plane planes[6],
                                 const uint8_t plane_index,
                                 Triangle3D *const restrict triangle,
                                 Triangle3DArray *const restrict array,
                                 Triangle3DArena *const restrict arena) {
    assert(planes != NULL);
    assert(plane_index <= 6);
    assert(triangle != NULL);
    assert(array != NULL);
    assert(arena != NULL);

    if (plane_index == 6) {
        triangle3D_array_push(array, triangle);
        return;
    }

    const Plane *plane = &planes[plane_index];

    const bool v1_in = v3f_dot(plane->normal, triangle->v1.xyz) >= plane->d;
    const bool v2_in = v3f_dot(plane->normal, triangle->v2.xyz) >= plane->d;
    const bool v3_in = v3f_dot(plane->normal, triangle->v3.xyz) >= plane->d;

    if (v1_in) {
        if (v2_in) {
            if (v3_in) {
                planes_clip_triangle(planes, plane_index + 1, triangle, array,
                                     arena);
                return;
            }

            const v4f intersect_v2_v3 =
                plane_intersect(plane, triangle->v2, triangle->v3);
            const v4f intersect_v1_v3 =
                plane_intersect(plane, triangle->v1, triangle->v3);

            const v2f uv_v2_v3 = v2f_lerp(
                triangle->uv2, triangle->uv3,
                get_interpolation(triangle->v2.xyz, intersect_v2_v3.xyz,
                                  triangle->v3.xyz));
            const v2f uv_v1_v3 = v2f_lerp(
                triangle->uv1, triangle->uv3,
                get_interpolation(triangle->v1.xyz, intersect_v1_v3.xyz,
                                  triangle->v3.xyz));

            planes_clip_triangle(
                planes, plane_index + 1,
                triangle3D_init_v4f(
                    &triangle->v1, &triangle->v2, &intersect_v2_v3,
                    triangle->uv1, triangle->uv2, uv_v2_v3,
                    triangle->edges &
                        (TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3 |
                         TRIANGLE_EDGE_V1_V2_FAR | TRIANGLE_EDGE_V2_V3_FAR),
                    triangle->texture, triangle->color, arena),
                array, arena);

            triangle->v2 = intersect_v2_v3;
            triangle->v3 = intersect_v1_v3;
            triangle->uv2 = uv_v2_v3;
            triangle->uv3 = uv_v1_v3;
            triangle->edges &= TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V3_V1_FAR;
            planes_clip_triangle(planes, plane_index + 1, triangle, array,
                                 arena);
            return;
        }

        if (v3_in) {
            const v4f intersect_v1_v2 =
                plane_intersect(plane, triangle->v1, triangle->v2);
            const v4f intersect_v2_v3 =
                plane_intersect(plane, triangle->v2, triangle->v3);

            const v2f uv_v1_v2 = v2f_lerp(
                triangle->uv1, triangle->uv2,
                get_interpolation(triangle->v1.xyz, intersect_v1_v2.xyz,
                                  triangle->v2.xyz));
            const v2f uv_v2_v3 = v2f_lerp(
                triangle->uv2, triangle->uv3,
                get_interpolation(triangle->v2.xyz, intersect_v2_v3.xyz,
                                  triangle->v3.xyz));

            planes_clip_triangle(
                planes, plane_index + 1,
                triangle3D_init_v4f(
                    &triangle->v1, &intersect_v1_v2, &triangle->v3,
                    triangle->uv1, uv_v1_v2, triangle->uv3,
                    triangle->edges &
                        (TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V3_V1 |
                         TRIANGLE_EDGE_V1_V2_FAR | TRIANGLE_EDGE_V3_V1_FAR),
                    triangle->texture, triangle->color, arena),
                array, arena);

            triangle->v1 = intersect_v1_v2;
            triangle->v2 = intersect_v2_v3;
            triangle->uv1 = uv_v1_v2;
            triangle->uv2 = uv_v2_v3;
            triangle->edges &= TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V2_V3_FAR;
            planes_clip_triangle(planes, plane_index + 1, triangle, array,
                                 arena);
            return;
        }

        const v4f intersect_v1_v2 =
            plane_intersect(plane, triangle->v1, triangle->v2);
        const v4f intersect_v1_v3 =
            plane_intersect(plane, triangle->v1, triangle->v3);

        const v2f uv_v1_v2 =
            v2f_lerp(triangle->uv1, triangle->uv2,
                     get_interpolation(triangle->v1.xyz, intersect_v1_v2.xyz,
                                       triangle->v2.xyz));
        const v2f uv_v1_v3 =
            v2f_lerp(triangle->uv1, triangle->uv3,
                     get_interpolation(triangle->v1.xyz, intersect_v1_v3.xyz,
                                       triangle->v3.xyz));

        triangle->v2 = intersect_v1_v2;
        triangle->v3 = intersect_v1_v3;
        triangle->uv2 = uv_v1_v2;
        triangle->uv3 = uv_v1_v3;
        triangle->edges &= (TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V3_V1 |
                            TRIANGLE_EDGE_V1_V2_FAR | TRIANGLE_EDGE_V3_V1_FAR);
        planes_clip_triangle(planes, plane_index + 1, triangle, array, arena);
        return;
    }

    if (v2_in) {
        if (v3_in) {
            const v4f intersect_v1_v2 =
                plane_intersect(plane, triangle->v1, triangle->v2);
            const v4f intersect_v1_v3 =
                plane_intersect(plane, triangle->v1, triangle->v3);

            const v2f uv_v1_v2 = v2f_lerp(
                triangle->uv1, triangle->uv2,
                get_interpolation(triangle->v1.xyz, intersect_v1_v2.xyz,
                                  triangle->v2.xyz));
            const v2f uv_v1_v3 = v2f_lerp(
                triangle->uv1, triangle->uv3,
                get_interpolation(triangle->v1.xyz, intersect_v1_v3.xyz,
                                  triangle->v3.xyz));

            planes_clip_triangle(
                planes, plane_index + 1,
                triangle3D_init_v4f(
                    &intersect_v1_v2, &triangle->v2, &triangle->v3, uv_v1_v2,
                    triangle->uv2, triangle->uv3,
                    triangle->edges &
                        (TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3 |
                         TRIANGLE_EDGE_V1_V2_FAR | TRIANGLE_EDGE_V2_V3_FAR),
                    triangle->texture, triangle->color, arena),
                array, arena);

            triangle->v1 = intersect_v1_v3;
            triangle->v2 = intersect_v1_v2;
            triangle->uv1 = uv_v1_v3;
            triangle->uv2 = uv_v1_v2;
            triangle->edges &= TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V3_V1_FAR;
            planes_clip_triangle(planes, plane_index + 1, triangle, array,
                                 arena);
            return;
        }
        const v4f intersect_v1_v2 =
            plane_intersect(plane, triangle->v1, triangle->v2);
        const v4f intersect_v2_v3 =
            plane_intersect(plane, triangle->v2, triangle->v3);

        const v2f uv_v1_v2 =
            v2f_lerp(triangle->uv1, triangle->uv2,
                     get_interpolation(triangle->v1.xyz, intersect_v1_v2.xyz,
                                       triangle->v3.xyz));
        const v2f uv_v2_v3 =
            v2f_lerp(triangle->uv2, triangle->uv3,
                     get_interpolation(triangle->v2.xyz, intersect_v2_v3.xyz,
                                       triangle->v3.xyz));

        triangle->v1 = intersect_v1_v2;
        triangle->v3 = intersect_v2_v3;
        triangle->uv1 = uv_v1_v2;
        triangle->uv3 = uv_v2_v3;
        triangle->edges &= (TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3 |
                            TRIANGLE_EDGE_V1_V2_FAR | TRIANGLE_EDGE_V2_V3_FAR);
        planes_clip_triangle(planes, plane_index + 1, triangle, array, arena);
        return;
    }

    if (v3_in) {
        const v4f intersect_v2_v3 =
            plane_intersect(plane, triangle->v2, triangle->v3);
        const v4f intersect_v1_v3 =
            plane_intersect(plane, triangle->v1, triangle->v3);

        const v2f uv_v2_v3 =
            v2f_lerp(triangle->uv2, triangle->uv3,
                     get_interpolation(triangle->v2.xyz, intersect_v2_v3.xyz,
                                       triangle->v3.xyz));
        const v2f uv_v1_v3 =
            v2f_lerp(triangle->uv1, triangle->uv3,
                     get_interpolation(triangle->v1.xyz, intersect_v1_v3.xyz,
                                       triangle->v3.xyz));

        triangle->v1 = intersect_v1_v3;
        triangle->v2 = intersect_v2_v3;
        triangle->uv1 = uv_v1_v3;
        triangle->uv2 = uv_v2_v3;
        triangle->edges &= (TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V3_V1 |
                            TRIANGLE_EDGE_V2_V3_FAR | TRIANGLE_EDGE_V3_V1_FAR);
        planes_clip_triangle(planes, plane_index + 1, triangle, array, arena);
        return;
    }
}

[[gnu::nonnull]]
static inline void clip_triangle(const Camera *const restrict camera,
                                 Triangle3D *const restrict triangle,
                                 Triangle3DArray *const restrict array,
                                 Triangle3DArena *const restrict arena) {
    assert(camera != NULL);
    assert(triangle != NULL);
    assert(array != NULL);
    assert(arena != NULL);

    const float d = tanf(CAMERA_FOV / 2.0f);
    const float a = atanf(d / CHARACTER_RATIO);
    const float b = atanf(d * camera->aspect_ratio);
    const float cos_a = cosf(a);
    const float sin_a = sinf(a);
    const float cos_b = cosf(b);
    const float sin_b = sinf(b);
    const Plane planes[6] = {
        // near
        {{0.0f, 0.0f, 1.0f}, CAMERA_Z_NEAR},
        // far
        {{0.0f, 0.0f, -1.0f}, -CAMERA_Z_FAR},
        // top
        {{0.0f, -cos_a, sin_a}, 0.0f},
        // bottom
        {{0.0f, cos_a, sin_a}, 0.0f},
        // left
        {{cos_b, 0.0f, sin_b}, 0.0f},
        // right
        {{-cos_b, 0.0f, sin_b}, 0.0f},
    };
    planes_clip_triangle(planes, 0, triangle, array, arena);
}

[[gnu::nonnull]] [[gnu::returns_nonnull]]
static inline Triangle3DArray *mesh_get_viewed_triangles(
    const Mesh *const restrict self, const Camera *const restrict camera,
    Triangle3DArena *const restrict arena) {
    assert(self != NULL);
    assert(camera != NULL);
    assert(arena != NULL);

    Triangle3DArray *const array =
        triangle3D_array_create(self->triangles->length);

    v3f *view_vertices = mesh_get_viewed_vertices(self, camera);

    for (size_t i = 0; i < self->triangles->length; ++i) {
        const TriangleIndex *const triangle_index = &self->triangles->array[i];
        Triangle3D *const triangle = triangle3D_init_v3f(
            &view_vertices[triangle_index->v1],
            &view_vertices[triangle_index->v2],
            &view_vertices[triangle_index->v3], triangle_index->uv1,
            triangle_index->uv2, triangle_index->uv3, triangle_index->edges,
            triangle_index->texture, triangle_index->color, arena);
        const v3f triangle_normal = triangle3D_get_normal(triangle);
        if (v3f_dot(triangle->v1.xyz, triangle_normal) < 0.0f) {
            clip_triangle(camera, triangle, array, arena);
        }
    }

    free(view_vertices);

    return array;
}

void mesh_render(const Mesh *const restrict self,
                 const Camera *const restrict camera,
                 const Viewport *const restrict viewport) {
    assert(self != NULL);
    assert(camera != NULL);
    assert(viewport != NULL);

    static const char shade_char[] = {'.', ';', '!'};

    Triangle3DArena *const arena =
        triangle3D_arena_create(2 * self->triangles->length);

    Triangle3DArray *const viewed_triangles =
        mesh_get_viewed_triangles(self, camera, arena);

    m4f camera_rotation_matrix;
    camera_get_rotation_matrix(camera, camera_rotation_matrix);
    const v4f light = mul_m4f_v3f(camera_rotation_matrix,
                                  v3f_normalize((v3f){3.0f, 2.0f, -1.0f}));

    for (size_t j = 0; j < viewed_triangles->length; ++j) {
        Triangle3D *triangle = viewed_triangles->array[j];
        const float triangle_distance_squared =
            min3_float(v3f_norm_squared(triangle->v1.xyz),
                       v3f_norm_squared(triangle->v2.xyz),
                       v3f_norm_squared(triangle->v3.xyz));

        if (triangle_distance_squared >
            MESH_SHADOW_DISTANCE * MESH_SHADOW_DISTANCE) {
            triangle->texture = NULL;
            triangle->color = MESH_SHADOW_COLOR;
            triangle->shade = shade_char[0];
        } else {
            // lighting
            const uint8_t shade_index =
                sizeof(shade_char) *
                clamp_float(
                    v3f_dot(light.xyz,
                            v3f_normalize(triangle3D_get_normal(triangle))),
                    0.0f, 0.999f);
            triangle->shade = shade_char[shade_index];
        }

        if (triangle_distance_squared >=
            MESH_OUTLINE_MAX_DISTANCE * MESH_OUTLINE_MAX_DISTANCE) {
            if (triangle_distance_squared >=
                MESH_FAR_OUTLINE_MAX_DISTANCE * MESH_FAR_OUTLINE_MAX_DISTANCE) {
                triangle->edges = 0;
            } else {
                triangle->edges &= TRIANGLE_EDGE_V1_V2_FAR |
                                   TRIANGLE_EDGE_V2_V3_FAR |
                                   TRIANGLE_EDGE_V3_V1_FAR;
            }
        }

        triangle->v1 = mul_m4f_v3f(camera->projection_matrix, triangle->v1.xyz);
        triangle->v2 = mul_m4f_v3f(camera->projection_matrix, triangle->v2.xyz);
        triangle->v3 = mul_m4f_v3f(camera->projection_matrix, triangle->v3.xyz);

        triangle->v1.x = viewport->x_offset +
                         viewport->width * (triangle->v1.x + 1.0f) / 2.0f;
        triangle->v2.x = viewport->x_offset +
                         viewport->width * (triangle->v2.x + 1.0f) / 2.0f;
        triangle->v3.x = viewport->x_offset +
                         viewport->width * (triangle->v3.x + 1.0f) / 2.0f;
        triangle->v1.y = viewport->y_offset +
                         viewport->height * (-triangle->v1.y + 1.0f) / 2.0f;
        triangle->v2.y = viewport->y_offset +
                         viewport->height * (-triangle->v2.y + 1.0f) / 2.0f;
        triangle->v3.y = viewport->y_offset +
                         viewport->height * (-triangle->v3.y + 1.0f) / 2.0f;

        window_render_triangle(triangle);
    }

    array_destroy((Array *)viewed_triangles);
    triangle3D_arena_destroy(arena);
}
