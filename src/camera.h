#pragma once

#include "vec.h"

#define RETURNS_NONNULL __attribute__((returns_nonnull))

typedef struct {
    v3f position;
    float yaw, pitch;
    m4f projection_matrix;
    float aspect_ratio;
} Camera;

Camera *camera_create(const v3f position, const float yaw, const float pitch,
                      const float aspect_ratio) RETURNS_NONNULL;

void camera_destroy(Camera *const self) NONNULL();

void camera_get_view_matrix(const Camera *const restrict self, m4f view_matrix)
    NONNULL();

void camera_get_rotation_matrix(const Camera *const self, m4f rotation_matrix)
    NONNULL();

void camera_get_projection_matrix(const Camera *const self,
                                  m4f projection_matrix) NONNULL();

v3f camera_get_look_direction(const Camera *const self) NONNULL();

void camera_set_aspect_ratio(Camera *const self, const float aspect_ratio)
    NONNULL(1);

void camera_rotate(Camera *const self, const v2f rotation) NONNULL(1);
