#include "camera.h"

#include <assert.h>

#include "config.h"
#include "vec.h"

[[gnu::nonnull]]
static void camera_update_projection_matrix(Camera *const self) {
    assert(self != NULL);

    const float d = 1 / tanf(CAMERA_FOV * 0.5f);
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

void camera_init(Camera *const self, const v3f position, const float yaw,
                 const float pitch, const float aspect_ratio) {
    assert(self != NULL);
    self->position = position;
    self->yaw = yaw;
    self->pitch = pitch;
    self->aspect_ratio = aspect_ratio;
    camera_update_projection_matrix(self);

    self->frustum_planes_in_camera_space.near.normal = (v3f){0.0f, 0.0f, 1.0f};
    self->frustum_planes_in_camera_space.near.distance = CAMERA_Z_NEAR;
    self->frustum_planes_in_camera_space.far.normal = (v3f){0.0f, 0.0f, -1.0f};
    self->frustum_planes_in_camera_space.far.distance = -CAMERA_Z_FAR;
    self->frustum_planes_in_camera_space.bottom.distance = 0.0f;
    self->frustum_planes_in_camera_space.top.distance = 0.0f;
    self->frustum_planes_in_camera_space.left.distance = 0.0f;
    self->frustum_planes_in_camera_space.right.distance = 0.0f;
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

v3f camera_get_forward_direction(const Camera *const self) {
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
    if (self->pitch > M_PI_2) {
        self->pitch = M_PI_2;
    } else if (self->pitch < -M_PI_2) {
        self->pitch = -M_PI_2;
    }
    self->yaw = fmodf(self->yaw + rotation.x, 2.0f * M_PI);
}

[[gnu::nonnull(1)]]
static inline void camera_update_frustum_planes_in_camera_space(
    Camera *const self, const float cos_a, const float sin_a, const float cos_b,
    const float sin_b) {
    assert(self != NULL);
    // Top and bottom plane are constante so no need to update them.
    // Same for the other plane distance.
    self->frustum_planes_in_camera_space.top.normal =
        (v3f){0.0f, -cos_a, sin_a};
    self->frustum_planes_in_camera_space.bottom.normal =
        (v3f){0.0f, cos_a, sin_a};
    self->frustum_planes_in_camera_space.left.normal =
        (v3f){cos_b, 0.0f, sin_b};
    self->frustum_planes_in_camera_space.right.normal =
        (v3f){-cos_b, 0.0f, sin_b};
}

void camera_update_frustum_planes(Camera *const self) {
    assert(self != NULL);

    const float d = tanf(CAMERA_FOV * 0.5f);
    const float a = atanf(d / CHARACTER_RATIO);
    const float b = atanf(d * self->aspect_ratio);
    const float cos_a = cosf(a);
    const float sin_a = sinf(a);
    const float cos_b = cosf(b);
    const float sin_b = sinf(b);

    const v3f world_up = {0.0f, 1.0f, 0.0f};
    const v3f forward = camera_get_forward_direction(self);
    const v3f right = v3f_normalize(v3f_cross_product(forward, world_up));
    const v3f up = v3f_normalize(v3f_cross_product(right, forward));

    const v3f forward_mul_sin_a = v3f_mul(forward, sin_a);
    const v3f forward_mul_sin_b = v3f_mul(forward, sin_b);

    {
        const v3f near_normal = forward;
        self->frustum_planes.near.normal = near_normal;
        self->frustum_planes.near.distance =
            v3f_dot(near_normal,
                    v3f_add(self->position, v3f_mul(forward, CAMERA_Z_NEAR)));
    }

    {
        const v3f far_normal = v3f_mul(forward, -1.0f);
        self->frustum_planes.far.normal = far_normal;
        self->frustum_planes.far.distance =
            v3f_dot(far_normal,
                    v3f_add(self->position, v3f_mul(forward, CAMERA_Z_FAR)));
    }

    {
        const v3f top_normal =
            v3f_normalize(v3f_add(v3f_mul(up, -cos_a), forward_mul_sin_a));
        self->frustum_planes.top.normal = top_normal;
        self->frustum_planes.top.distance = v3f_dot(top_normal, self->position);
    }

    {
        const v3f bottom_normal =
            v3f_normalize(v3f_add(v3f_mul(up, cos_a), forward_mul_sin_a));
        self->frustum_planes.bottom.normal = bottom_normal;
        self->frustum_planes.bottom.distance =
            v3f_dot(bottom_normal, self->position);
    }

    {
        const v3f left_normal =
            v3f_normalize(v3f_add(v3f_mul(right, -cos_b), forward_mul_sin_b));
        self->frustum_planes.left.normal = left_normal;
        self->frustum_planes.left.distance =
            v3f_dot(left_normal, self->position);
    }

    {
        const v3f right_normal =
            v3f_normalize(v3f_add(v3f_mul(right, cos_b), forward_mul_sin_b));
        self->frustum_planes.right.normal = right_normal;
        self->frustum_planes.right.distance =
            v3f_dot(right_normal, self->position);
    }

    camera_update_frustum_planes_in_camera_space(self, cos_a, sin_a, cos_b,
                                                 sin_b);
}

[[gnu::nonnull]]
static bool is_aabb_forward_plane(const Aabb *const restrict aabb,
                                  const Plane *const restrict plane) {
    assert(aabb != NULL);
    assert(plane != NULL);
    const v3f extents = v3f_mul(aabb->size, 0.5f);
    const float r = v3f_dot(extents, v3f_abs(plane->normal));
    const v3f center = v3f_add(aabb->position, extents);
    return -r <= v3f_dot(plane->normal, center) - plane->distance;
}

bool camera_aabb_in_frustum(const Camera *const restrict self,
                            const Aabb *const restrict aabb) {
    assert(self != NULL);
    assert(aabb != NULL);
    return (is_aabb_forward_plane(aabb, &self->frustum_planes.planes[0]) &&
            is_aabb_forward_plane(aabb, &self->frustum_planes.planes[1]) &&
            is_aabb_forward_plane(aabb, &self->frustum_planes.planes[2]) &&
            is_aabb_forward_plane(aabb, &self->frustum_planes.planes[3]) &&
            is_aabb_forward_plane(aabb, &self->frustum_planes.planes[4]) &&
            is_aabb_forward_plane(aabb, &self->frustum_planes.planes[5]));
}
