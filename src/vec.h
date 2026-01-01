#pragma once

#include <math.h>
#include <stdbool.h>

#include "utils.h"

typedef struct {
    int x, y;
} v2i;

typedef struct {
    float x, y;
} v2f;

typedef struct {
    int x, y, z;
} v3i;

typedef struct {
    float x, y, z;
} v3f;

typedef union {
    struct {
        float x, y, z, w;
    };
    v3f xyz;
} v4f;

typedef float m4f[16];

[[gnu::nonnull]]
void mul_m4f_m4f(const m4f mat1, const m4f mat2, m4f output);
[[gnu::nonnull(1)]]
v4f mul_m4f_v3f(const m4f mat, const v3f vec);

[[gnu::nonnull(1)]]
void m4f_rotation_x(m4f rotation_matrix, const float angle);
[[gnu::nonnull(1)]]
void m4f_rotation_y(m4f rotation_matrix, const float angle);
[[gnu::nonnull(1)]]
void m4f_rotation_z(m4f rotation_matrix, const float angle);

static inline v2i v2i_add(const v2i v1, const v2i v2) {
    return (v2i){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
    };
}

static inline v2i v2i_add_i(const v2i vec, const int value) {
    return (v2i){
        .x = vec.x + value,
        .y = vec.y + value,
    };
}

static inline v2i v2i_sub(const v2i v1, const v2i v2) {
    return (v2i){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
    };
}

static inline v2f v2i_mul_f(const v2i vec, const float value) {
    return (v2f){
        .x = vec.x * value,
        .y = vec.y * value,
    };
}

static inline bool v2i_equals(const v2i v1, const v2i v2) {
    return v1.x == v2.x && v1.y == v2.y;
}

static inline v2i v2f_floor(const v2f vec) {
    return (v2i){
        .x = floorf(vec.x),
        .y = floorf(vec.y),
    };
}

static inline v2f v2f_sub(const v2f v1, const v2f v2) {
    return (v2f){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
    };
}

static inline v2f v2f_sub_v2i(const v2f v1, const v2i v2) {
    return (v2f){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
    };
}

static inline v2f v2f_mul(const v2f vec, const float value) {
    return (v2f){
        .x = vec.x * value,
        .y = vec.y * value,
    };
}

static inline v2f v2f_div(const v2f vec, const float value) {
    return v2f_mul(vec, 1.0f / value);
}

static inline float v2f_dot(const v2f v1, const v2f v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

static inline float v2f_norm(const v2f vec) {
    return sqrtf(v2f_dot(vec, vec));
}

static inline float v2f_get_determinant(const v2f a, const v2f b, const v2f c) {
    const v2f ab = v2f_sub(b, a);
    const v2f ac = v2f_sub(c, a);

    return (ab.x * ac.y) - (ab.y * ac.x);
}

static inline v2f v2f_lerp(const v2f v1, const v2f v2, const float t) {
    return (v2f){
        .x = lerp(v1.x, v2.x, t),
        .y = lerp(v1.y, v2.y, t),
    };
}

static inline float v3f_dot(const v3f v1, const v3f v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

static inline float v3f_norm_squared(const v3f vec) {
    return v3f_dot(vec, vec);
}

static inline v3f v3f_mul(const v3f vec, const float value) {
    return (v3f){
        .x = vec.x * value,
        .y = vec.y * value,
        .z = vec.z * value,
    };
}

static inline v3f v3f_div(const v3f vec, const float value) {
    return v3f_mul(vec, 1.0f / value);
}

static inline v3f v3f_normalize(const v3f vec) {
    const float norm_squared = v3f_norm_squared(vec);
    if (norm_squared == 0.0f) return (v3f){0.0f, 0.0f, 0.0f};
    return v3f_div(vec, sqrtf(norm_squared));
}

static inline v3f v3f_add(const v3f v1, const v3f v2) {
    return (v3f){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
        .z = v1.z + v2.z,
    };
}

static inline v3f v3f_sub(const v3f v1, const v3f v2) {
    return (v3f){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
        .z = v1.z - v2.z,
    };
}

static inline v3f v3f_cross_product(const v3f v1, const v3f v2) {
    return (v3f){
        .x = v1.y * v2.z - v1.z * v2.y,
        .y = v1.z * v2.x - v1.x * v2.z,
        .z = v1.x * v2.y - v1.y * v2.x,
    };
}

static inline v4f v3f_to_v4f(const v3f vec) {
    return (v4f){
        .x = vec.x,
        .y = vec.y,
        .z = vec.z,
        1.0f,
    };
}

static inline v4f v4f_sub(const v4f v1, const v4f v2) {
    return (v4f){
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
        .z = v1.z - v2.z,
        .w = v1.w - v2.w,
    };
}

static inline v4f v4f_add_v3f(const v4f v1, const v3f v2) {
    return (v4f){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
        .z = v1.z + v2.z,
        .w = v1.w,
    };
}
