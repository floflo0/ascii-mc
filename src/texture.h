#pragma once

#include "color.h"

#define TEXTURE_SIZE 16

typedef Color Texture;

[[gnu::nonnull(1)]]
Color texture_get(const Texture *const texture, const float u, const float v);
