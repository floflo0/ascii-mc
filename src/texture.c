#include "texture.h"

#include <assert.h>
#include <stddef.h>

#include "utils.h"

Color texture_get(const Texture *const texture, const float u, const float v) {
    assert(texture != NULL);

    const int texture_x = clamp_int(u * TEXTURE_SIZE, 0, TEXTURE_SIZE - 1);
    const int texture_y = clamp_int(v * TEXTURE_SIZE, 0, TEXTURE_SIZE - 1);

    const Color color = texture[texture_y * TEXTURE_SIZE + texture_x];
    assert(color < COLOR_COUNT);
    return color;
}
