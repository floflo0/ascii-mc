#pragma once

#include <stdint.h>

typedef struct {
    int x_offset;
    int y_offset;
    int width;
    int height;
} Viewport;

[[gnu::nonnull(1)]]
void viewport_from_player_index(Viewport *const viewport,
                                const int8_t player_index,
                                const uint8_t number_players);
