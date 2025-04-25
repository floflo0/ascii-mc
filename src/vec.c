#include "vec.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

void mul_m4f_m4f(const m4f mat1, const m4f mat2, m4f output) {
    assert(mat1 != NULL);
    assert(mat2 != NULL);
    assert(output != NULL);
    for (uint8_t i = 0; i < 4; ++i) {
        for (uint8_t j = 0; j < 4; ++j) {
            output[i * 4 + j] = 0.0f;
            const uint8_t index = i * 4 + j;
            for (uint8_t k = 0; k < 4; ++k) {
                output[index] += mat1[i * 4 + k] * mat2[j + k * 4];
            }
        }
    }
}

v3f mul_m4f_v3f(const m4f mat, const v3f vec) {
    assert(mat != NULL);

    v3f output = {
        .x = mat[0] * vec.x + mat[1] * vec.y + mat[2] * vec.z + mat[3],
        .y = mat[4] * vec.x + mat[5] * vec.y + mat[6] * vec.z + mat[7],
        .z = mat[8] * vec.x + mat[9] * vec.y + mat[10] * vec.z + mat[11],
    };

    const float w =
        mat[12] * vec.x + mat[13] * vec.y + mat[14] * vec.z + mat[15];

    if (w) {
        output.x /= w;
        output.y /= w;
        // output.z /= w;
    }

    return output;
}

void m4f_rotation_x(m4f rotation_matrix, const float angle) {
    assert(rotation_matrix != NULL);

    const float cos_angle = cosf(angle);
    const float sin_angle = sinf(angle);

    rotation_matrix[0] = 1.0f;
    rotation_matrix[1] = 0.0f;
    rotation_matrix[2] = 0.0f;
    rotation_matrix[3] = 0.0f;

    rotation_matrix[4] = 0.0f;
    rotation_matrix[5] = cos_angle;
    rotation_matrix[6] = sin_angle;
    rotation_matrix[7] = 0.0f;

    rotation_matrix[8] = 0.0f;
    rotation_matrix[9] = -sin_angle;
    rotation_matrix[10] = cos_angle;
    rotation_matrix[11] = 0.0f;

    rotation_matrix[12] = 0.0f;
    rotation_matrix[13] = 0.0f;
    rotation_matrix[14] = 0.0f;
    rotation_matrix[15] = 1.0f;
}

void m4f_rotation_y(m4f rotation_matrix, const float angle) {
    assert(rotation_matrix != NULL);

    const float cos_angle = cosf(angle);
    const float sin_angle = sinf(angle);

    rotation_matrix[0] = cos_angle;
    rotation_matrix[1] = 0.0f;
    rotation_matrix[2] = -sin_angle;
    rotation_matrix[3] = 0.0f;

    rotation_matrix[4] = 0.0f;
    rotation_matrix[5] = 1.0f;
    rotation_matrix[6] = 0.0f;
    rotation_matrix[7] = 0.0f;

    rotation_matrix[8] = sin_angle;
    rotation_matrix[9] = 0.0f;
    rotation_matrix[10] = cos_angle;
    rotation_matrix[11] = 0.0f;

    rotation_matrix[12] = 0.0f;
    rotation_matrix[13] = 0.0f;
    rotation_matrix[14] = 0.0f;
    rotation_matrix[15] = 1.0f;
}

void m4f_rotation_z(m4f rotation_matrix, const float angle) {
    assert(rotation_matrix != NULL);

    const float cos_angle = cosf(angle);
    const float sin_angle = sinf(angle);

    rotation_matrix[0] = cos_angle;
    rotation_matrix[1] = -sin_angle;
    rotation_matrix[2] = 0.0f;
    rotation_matrix[3] = 0.0f;

    rotation_matrix[4] = sin_angle;
    rotation_matrix[5] = cos_angle;
    rotation_matrix[6] = 0.0f;
    rotation_matrix[7] = 0.0f;

    rotation_matrix[8] = 0.0f;
    rotation_matrix[9] = 0.0f;
    rotation_matrix[10] = 1.0f;
    rotation_matrix[11] = 0.0f;

    rotation_matrix[12] = 0.0f;
    rotation_matrix[13] = 0.0f;
    rotation_matrix[14] = 0.0f;
    rotation_matrix[15] = 1.0f;
}
