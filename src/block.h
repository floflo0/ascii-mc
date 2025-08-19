#pragma once

#include <stdint.h>

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

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

#define BLOCKS                                                            \
    ONE_COLOR_BLOCK(AIR, "Air", COLOR_WHITE)                              \
    BLOCK(DIRT, "Dirt", COLOR_DARK_GREEN, COLOR_DARK_RED, COLOR_DARK_RED, \
          NULL, dirt_side_texture, NULL)                                  \
    ONE_COLOR_BLOCK(STONE, "Stone", COLOR_LIGHT_GREY)                     \
    ONE_COLOR_BLOCK(SAND, "Sand", COLOR_YELLOW)                           \
    ONE_COLOR_BLOCK(SNOW, "Snow", COLOR_WHITE)                            \
    ONE_COLOR_BLOCK(BEDROCK, "Bedrock", COLOR_DARK_GREY)

typedef enum : uint8_t {
#define BLOCK(name, ...) BLOCK_TYPE_##name,
    BLOCKS
#undef BLOCK
        BLOCK_TYPE_COUNT,
} BlockType;

typedef struct {
    BlockType type;
} Block;

void block_init(Block *const block, const BlockType type) NONNULL(1);

const char *block_get_name(const BlockType type);
