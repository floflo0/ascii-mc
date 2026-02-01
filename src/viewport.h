#pragma once

#include <stdint.h>

#include "viewport_defs.h"

[[gnu::nonnull(1)]]
void viewport_from_player_index(Viewport *const viewport,
                                const int8_t player_index,
                                const uint8_t number_players,
                                const float character_ratio);
