#include "perlin_noise.h"

#include "utils.h"
#include "vec.h"

static inline float fade(const float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static uint32_t pcg_hash(const uint32_t input) {
    const uint32_t state = input * 747796405u + 2891336453u;
    const uint32_t word =
        ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

static v2f grad(const v2i p, const uint32_t seed) {
    const float angle =
        2 * M_PI * pcg_hash(pcg_hash(p.x) + p.y + seed) / UINT32_MAX;
    return (v2f){cosf(angle), sinf(angle)};
}

static inline float noise(const v2f p, const uint32_t seed) {
    const v2i p0 = v2f_floor(p);
    const v2i p1 = {p0.x + 1, p0.y};
    const v2i p2 = {p0.x, p0.y + 1};
    const v2i p3 = v2i_add(p0, (v2i){1, 1});

    const float fade_x = fade(p.x - p0.x);
    const float fade_y = fade(p.y - p0.y);

    return lerp(lerp(v2f_dot(v2f_sub_v2i(p, p0), grad(p0, seed)),
                     v2f_dot(v2f_sub_v2i(p, p1), grad(p1, seed)), fade_x),
                lerp(v2f_dot(v2f_sub_v2i(p, p2), grad(p2, seed)),
                     v2f_dot(v2f_sub_v2i(p, p3), grad(p3, seed)), fade_x),
                fade_y);
}

float perlin_noise(const v2i position, const uint32_t seed, float frequency,
                   const uint8_t depth) {
    float result = 0.0f;
    float amplitude = 1.0f;

    for (uint8_t i = 0; i < depth; ++i) {
        result += noise(v2i_mul_f(position, frequency), seed) * amplitude;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    return (result + 1.0f) / 2.0f;
}
