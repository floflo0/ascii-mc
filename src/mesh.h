#pragma once

#include "camera_defs.h"
#include "mesh_defs.h"
#include "viewport.h"

[[gnu::nonnull(1)]]
void mesh_init(Mesh *const self, const size_t preallocate_vertices_size,
               const size_t preallocate_triangles_size);

[[gnu::nonnull]]
void mesh_destroy(const Mesh *const self);

[[gnu::nonnull]]
void mesh_clear(Mesh *const self);

[[gnu::nonnull]]
void mesh_render(const Mesh *const restrict self,
                 const Camera *const restrict camera,
                 const Viewport *const restrict viewport);
