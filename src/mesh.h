#pragma once

#include "camera.h"
#include "triangle_index_array.h"
#include "v3f_array.h"
#include "viewport.h"

typedef struct {
    V3fArray *vertices;
    TriangleIndexArray *triangles;
} Mesh;

[[gnu::returns_nonnull]]
Mesh *mesh_create(const size_t preallocate_vertices_size,
                  const size_t preallocate_triangles_size);

[[gnu::nonnull]]
void mesh_destroy(Mesh *const mesh);

[[gnu::nonnull]]
void mesh_render(const Mesh *const restrict self,
                 const Camera *const restrict camera,
                 const Viewport *const restrict viewport);
