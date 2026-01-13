#pragma once

#include <stdint.h>

#include "vec_defs.h"

typedef enum : uint8_t {
    COLLISION_AXIS_X_PLUS,
    COLLISION_AXIS_X_MINUS,
    COLLISION_AXIS_Y_PLUS,
    COLLISION_AXIS_Y_MINUS,
    COLLISION_AXIS_Z_PLUS,
    COLLISION_AXIS_Z_MINUS,
} CollisionAxis;

typedef struct {
    v3f position;
    v3f size;
} Aabb;

[[gnu::nonnull(1, 4, 5)]]
bool aabb_collide_ray(const Aabb *const restrict aabb, const v3f ray_start,
                      const v3f ray_direction,
                      float *const restrict collision_time,
                      CollisionAxis *const restrict collision_axis);
