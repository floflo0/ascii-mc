#pragma once

#include <stdint.h>

// BLOCK(
//     name,
//     name_string,
//     top_color,
//     side_color,
//     bottom_color,
//     top_texture,
//     side_texture,
//     bottom_texture
// )

#define ONE_COLOR_BLOCK(name, name_string, color) \
    BLOCK(name, name_string, color, color, color, NULL, NULL, NULL)

#define ONE_TEXTURE_BLOCK(name, name_string, texture)                        \
    BLOCK(name, name_string, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, texture, \
          texture, texture)

#define BLOCKS                                                              \
    ONE_COLOR_BLOCK(AIR, "Air", COLOR_WHITE)                                \
    ONE_COLOR_BLOCK(BEDROCK, "Bedrock", COLOR_DARK_GREY)                    \
    BLOCK(GRASS, "Grass", COLOR_DARK_GREEN, COLOR_DARK_RED, COLOR_DARK_RED, \
          NULL, grass_side_texture, dirt_texture)                           \
    ONE_TEXTURE_BLOCK(DIRT, "Dirt", dirt_texture)                           \
    ONE_COLOR_BLOCK(STONE, "Stone", COLOR_LIGHT_GREY)                       \
    ONE_COLOR_BLOCK(SNOW, "Snow", COLOR_WHITE)                              \
    ONE_COLOR_BLOCK(SAND, "Sand", COLOR_YELLOW)                             \
    ONE_TEXTURE_BLOCK(AMETHYST_ORE, "Amethyst Ore", amethyst_ore_texture)   \
    ONE_TEXTURE_BLOCK(DIAMOND_ORE, "Diamond Ore", diamond_ore_texture)      \
    ONE_TEXTURE_BLOCK(EMERALD_ORE, "Emerald Ore", emerald_ore_texture)      \
    ONE_TEXTURE_BLOCK(RUBY_ORE, "Ruby Ore", ruby_ore_texture)               \
    ONE_TEXTURE_BLOCK(SAPPHIRE_ORE, "Sapphire Ore", sapphire_ore_texture)   \
    ONE_TEXTURE_BLOCK(TOPAZ_ORE, "Topaz Ore", topaz_ore_texture)

typedef enum : uint8_t {
#define BLOCK(name, ...) BLOCK_TYPE_##name,
    BLOCKS
#undef BLOCK
        BLOCK_TYPE_COUNT,
} BlockType;

typedef struct {
    BlockType type;
} Block;

[[gnu::nonnull(1)]]
void block_init(Block *const block, const BlockType type);

[[gnu::returns_nonnull]]
const char *block_get_name(const BlockType type);
