#include "camera.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "config.h"
#include "utils.h"

static void camera_update_projection_matrix(Camera *const self) NONNULL();

static void camera_update_projection_matrix(Camera *const self) {
    assert(self != NULL);

    const float d = 1 / tanf(CAMERA_FOV / 2.0f);
    const float l = CAMERA_Z_FAR / (CAMERA_Z_FAR - CAMERA_Z_NEAR);

    self->projection_matrix[0] = d / self->aspect_ratio;
    self->projection_matrix[1] = 0.0f;
    self->projection_matrix[2] = 0.0f;
    self->projection_matrix[3] = 0.0f;

    self->projection_matrix[4] = 0.0f;
    self->projection_matrix[5] = d * CHARACTER_RATIO;
    self->projection_matrix[6] = 0.0f;
    self->projection_matrix[7] = 0.0f;

    self->projection_matrix[8] = 0.0f;
    self->projection_matrix[9] = 0.0f;
    self->projection_matrix[10] = l;
    self->projection_matrix[11] = -l * CAMERA_Z_NEAR;

    self->projection_matrix[12] = 0.0f;
    self->projection_matrix[13] = 0.0f;
    self->projection_matrix[14] = 1.0f;
    self->projection_matrix[15] = 0.0f;
}

Camera *camera_create(const v3f position, const float yaw, const float pitch,
                      const float aspect_ratio) {
    Camera *const self =
        malloc_or_exit(sizeof(*self), "failed to create camera");

    self->position = position;
    self->yaw = yaw;
    self->pitch = pitch;
    self->aspect_ratio = aspect_ratio;

    camera_update_projection_matrix(self);

    return self;
}

void camera_destroy(Camera *const self) {
    assert(self != NULL);
    free(self);
}

void camera_get_rotation_matrix(const Camera *const self, m4f rotation_matrix) {
    assert(self != NULL);
    assert(rotation_matrix != NULL);
    m4f rotation_x_matrix;
    m4f_rotation_x(rotation_x_matrix, -self->pitch);
    m4f rotation_y_matrix;
    m4f_rotation_y(rotation_y_matrix, -self->yaw);
    mul_m4f_m4f(rotation_x_matrix, rotation_y_matrix, rotation_matrix);
}

void camera_get_view_matrix(const Camera *const self, m4f view_matrix) {
    assert(self != NULL);
    assert(view_matrix != NULL);
    m4f rotation_matrix;
    camera_get_rotation_matrix(self, rotation_matrix);
    const m4f translation_matrix = {
        1.0f, 0.0f, 0.0f, -self->position.x,
        0.0f, 1.0f, 0.0f, -self->position.y,
        0.0f, 0.0f, 1.0f, -self->position.z,
        0.0f, 0.0f, 0.0f, 0.0f,
    };
    mul_m4f_m4f(rotation_matrix, translation_matrix, view_matrix);
}

v3f camera_get_look_direction(const Camera *const self) {
    assert(self != NULL);
    const float cos_pitch = cosf(self->pitch);
    return (v3f){
        .x = -sinf(self->yaw) * cos_pitch,
        .y = sinf(self->pitch),
        .z = cosf(self->yaw) * cos_pitch,
    };
}

void camera_set_aspect_ratio(Camera *const self, const float aspect_ratio) {
    assert(self != NULL);
    self->aspect_ratio = aspect_ratio;
    camera_update_projection_matrix(self);
}

void camera_rotate(Camera *const self, const v2f rotation) {
    assert(self != NULL);
    self->pitch += rotation.y;
    if (self->pitch > M_PI / 2.0f) {
        self->pitch = M_PI / 2.0f;
    } else if (self->pitch < -M_PI / 2.0f) {
        self->pitch = -M_PI / 2.0f;
    }
    self->yaw = fmodf(self->yaw + rotation.x, 2.0f * M_PI);
}
