#pragma once

#include "camera.h"
#include "triangle_index_array.h"
#include "v3f_array.h"
#include "viewport.h"

typedef struct {
    V3fArray *vertices;
    TriangleIndexArray *triangles;
} Mesh;

Mesh *mesh_create(const size_t preallocate_vertices_size,
                  const size_t preallocate_triangles_size) RETURNS_NONNULL;
void mesh_destroy(Mesh *const mesh) NONNULL();
void mesh_render(const Mesh *const restrict self,
                 const Camera *const restrict camera,
                 const Viewport *const restrict viewport) NONNULL();
