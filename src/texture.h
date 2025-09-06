#pragma once

#include "color.h"

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

#define TEXTURE_SIZE 16

typedef Color Texture;

Color texture_get(const Texture *const texture, const float u, const float v)
    NONNULL(1);
