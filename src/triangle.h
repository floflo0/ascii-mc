#pragma once

#include <stddef.h>
#include <stdint.h>

#include "texture.h"
#include "vec.h"

#define RETURNS_NONNULL __attribute__((returns_nonnull))

#define TRIANGLE_EDGE_V1_V2 0x01
#define TRIANGLE_EDGE_V2_V3 0x02
#define TRIANGLE_EDGE_V3_V1 0x04
#define TRIANGLE_EDGE_V1_V2_FAR 0x08
#define TRIANGLE_EDGE_V2_V3_FAR 0x10
#define TRIANGLE_EDGE_V3_V1_FAR 0x20

typedef struct {
    v4f v1;
    v4f v2;
    v4f v3;
    v2f uv1;
    v2f uv2;
    v2f uv3;
    const Texture *texture;
    uint8_t edges;
    char shade;
    Color color;
} Triangle3D;

typedef struct {
    size_t v1;
    size_t v2;
    size_t v3;
    v2f uv1;
    v2f uv2;
    v2f uv3;
    const Texture *texture;
    Color color;
    uint8_t edges;
} TriangleIndex;

Triangle3D *triangle3D_init_v3f(const v3f *const restrict v1,
                                const v3f *const restrict v2,
                                const v3f *const restrict v3, const v2f uv1,
                                const v2f uv2, const v2f uv3,
                                const uint8_t edges,
                                const Texture *const texture, const Color color)
    NONNULL(1, 2, 3) RETURNS_NONNULL;
Triangle3D *triangle3D_init_v4f(const v4f *const restrict v1,
                                const v4f *const restrict v2,
                                const v4f *const restrict v3, const v2f uv1,
                                const v2f uv2, const v2f uv3,
                                const uint8_t edges,
                                const Texture *const texture, const Color color)
    NONNULL(1, 2, 3) RETURNS_NONNULL;
void triangle3D_destroy(Triangle3D *const triangle) NONNULL();
v3f triangle3D_get_normal(const Triangle3D *const triangle) NONNULL();
