#include "texture.h"

#include <assert.h>
#include <stddef.h>

#include "utils.h"

Color texture_get(const Texture *const texture, float u, float v) {
    assert(texture != NULL);

    u = clamp_float(u, 0.0f, 1.0f);
    v = clamp_float(v, 0.0f, 1.0f);

    const size_t texture_x = (size_t)(u * TEXTURE_SIZE);
    const size_t texture_y = (size_t)(v * TEXTURE_SIZE);

    return texture[texture_y * TEXTURE_SIZE + texture_x];
}
