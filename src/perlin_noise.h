#pragma once

#include <stdint.h>

#include "vec_defs.h"

float perlin_noise(const v2i position, const uint32_t seed, float frequency,
                   const uint8_t depth);
