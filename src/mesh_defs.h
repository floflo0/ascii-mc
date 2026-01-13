#pragma once

#include "triangle_index_array_defs.h"
#include "v3f_array_defs.h"

typedef struct {
    V3fArray vertices;
    TriangleIndexArray triangles;
} Mesh;
