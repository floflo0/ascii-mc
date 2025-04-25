#pragma once

#include <stdint.h>

#include "vec.h"

float perlin_noise(const v2i position, const uint32_t seed, float frequency,
                   const uint8_t depth);
