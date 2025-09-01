#pragma once

#include "texture.h"

#define DEFINE_TEXTURE(texture) \
    extern const Texture texture##_texture[TEXTURE_SIZE * TEXTURE_SIZE]

DEFINE_TEXTURE(amethyst_ore);
DEFINE_TEXTURE(diamond_ore);
DEFINE_TEXTURE(dirt);
DEFINE_TEXTURE(emerald_ore);
DEFINE_TEXTURE(grass_side);
DEFINE_TEXTURE(ruby_ore);
DEFINE_TEXTURE(sapphire_ore);
DEFINE_TEXTURE(topaz_ore);
