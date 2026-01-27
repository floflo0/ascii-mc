#pragma once

#include "collision_defs.h"

[[gnu::nonnull(1, 4, 5)]]
bool aabb_collide_ray(const Aabb *const restrict aabb, const v3f ray_start,
                      const v3f ray_direction,
                      float *const restrict collision_time,
                      CollisionAxis *const restrict collision_axis);
