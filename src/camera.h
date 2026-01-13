#pragma once

#include "camera_defs.h"

[[gnu::nonnull(1)]]
void camera_init(Camera *const self, const v3f position, const float yaw,
                 const float pitch, const float aspect_ratio);

[[gnu::nonnull]]
void camera_get_view_matrix(const Camera *const restrict self, m4f view_matrix);

[[gnu::nonnull]]
void camera_get_rotation_matrix(const Camera *const self, m4f rotation_matrix);

[[gnu::nonnull]]
v3f camera_get_look_direction(const Camera *const self);

[[gnu::nonnull(1)]]
void camera_set_aspect_ratio(Camera *const self, const float aspect_ratio);

[[gnu::nonnull(1)]]
void camera_rotate(Camera *const self, const v2f rotation);
