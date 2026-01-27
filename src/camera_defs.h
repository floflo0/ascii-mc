#pragma once

#include "vec_defs.h"

typedef struct {
    v3f normal;
    float distance;
} Plane;

typedef struct {
    m4f projection_matrix;
    v3f position;
    float yaw, pitch;
    float aspect_ratio;
    union {
        struct {
            Plane near;
            Plane bottom;
            Plane left;
            Plane right;
            Plane top;
            Plane far;
        };
        Plane planes[6];
    } frustum_planes_in_camera_space;
    union {
        struct {
            Plane near;
            Plane left;
            Plane right;
            Plane bottom;
            Plane top;
            Plane far;
        };
        Plane planes[6];
    } frustum_planes;
} Camera;
