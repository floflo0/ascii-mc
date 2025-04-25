#include "collision.h"

#include <assert.h>

#include "utils.h"

bool aabb_collide_ray(const Aabb *const restrict aabb, const v3f ray_start,
                      const v3f ray_direction,
                      float *const restrict collision_time,
                      CollisionAxis *const restrict collision_axis) {
    assert(aabb != NULL);
    assert(collision_time != NULL);
    assert(collision_axis != NULL);

    float entry_time_x = (aabb->position.x - ray_start.x) / ray_direction.x;
    float exit_time_x =
        ((aabb->position.x + aabb->size.x) - ray_start.x) / ray_direction.x;
    if (isnanf(entry_time_x) || isnanf(exit_time_x)) return false;
    if (entry_time_x > exit_time_x) {
        float tmp = entry_time_x;
        entry_time_x = exit_time_x;
        exit_time_x = tmp;
    }

    float entry_time_y = (aabb->position.y - ray_start.y) / ray_direction.y;
    float exit_time_y =
        ((aabb->position.y + aabb->size.y) - ray_start.y) / ray_direction.y;
    if (isnanf(entry_time_y) || isnanf(exit_time_y)) return false;
    if (entry_time_y > exit_time_y) {
        float tmp = entry_time_y;
        entry_time_y = exit_time_y;
        exit_time_y = tmp;
    }

    float entry_time_z = (aabb->position.z - ray_start.z) / ray_direction.z;
    float exit_time_z =
        ((aabb->position.z + aabb->size.z) - ray_start.z) / ray_direction.z;
    if (isnanf(entry_time_z) || isnanf(exit_time_z)) return false;
    if (entry_time_z > exit_time_z) {
        float tmp = entry_time_z;
        entry_time_z = exit_time_z;
        exit_time_z = tmp;
    }

    float entry_time;
    if (entry_time_x > entry_time_y) {
        if (entry_time_x > entry_time_z) {
            entry_time = entry_time_x;
            *collision_axis = ray_direction.x > 0.0f ? COLLISION_AXIS_X_MINUS
                                                     : COLLISION_AXIS_X_PLUS;
        } else {
            entry_time = entry_time_z;
            *collision_axis = ray_direction.z > 0.0f ? COLLISION_AXIS_Z_MINUS
                                                     : COLLISION_AXIS_Z_PLUS;
        }
    } else if (entry_time_y > entry_time_z) {
        entry_time = entry_time_y;
        *collision_axis = ray_direction.y > 0.0f ? COLLISION_AXIS_Y_MINUS
                                                 : COLLISION_AXIS_Y_PLUS;
    } else {
        entry_time = entry_time_z;
        *collision_axis = ray_direction.z > 0.0f ? COLLISION_AXIS_Z_MINUS
                                                 : COLLISION_AXIS_Z_PLUS;
    }
    const float exit_time = min3_float(exit_time_x, exit_time_y, exit_time_z);

    if (entry_time >= exit_time || entry_time < 0.0f || entry_time > 1.0f) {
        return false;
    }

    *collision_time = entry_time;

    return true;
}
