#pragma once

#include "vec_defs.h"

typedef struct {
    m4f projection_matrix;
    v3f position;
    float yaw, pitch;
    float aspect_ratio;
} Camera;
