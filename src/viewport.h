#pragma once

#include <stdint.h>

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

typedef struct {
    int x_offset;
    int y_offset;
    int width;
    int height;
} Viewport;

void viewport_from_player_index(Viewport *const viewport,
                                const int8_t player_index,
                                const uint8_t number_players) NONNULL(1);
