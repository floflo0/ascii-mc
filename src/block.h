#pragma once

#include <stdint.h>

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

// name, top face, other face
#define BLOCKS                                                \
    BLOCK(AIR, "Air", COLOR_WHITE, COLOR_WHITE)               \
    BLOCK(DIRT, "Dirt", COLOR_GREEN, COLOR_RED)               \
    BLOCK(STONE, "Stone", COLOR_LIGHT_GREY, COLOR_LIGHT_GREY) \
    BLOCK(SAND, "Sand", COLOR_YELLOW, COLOR_YELLOW)           \
    BLOCK(SNOW, "Snow", COLOR_WHITE, COLOR_WHITE)             \
    BLOCK(BEDROCK, "Bedrock", COLOR_DARK_GREY, COLOR_DARK_GREY)

typedef enum : uint8_t {
#define BLOCK(name, name_string, top_color, color) BLOCK_TYPE_##name,
    BLOCKS
#undef BLOCK
        BLOCK_TYPE_COUNT,
} BlockType;

typedef struct {
    BlockType type;
} Block;

void block_init(Block *const block, const BlockType type) NONNULL(1);

const char *block_get_name(const BlockType type);
