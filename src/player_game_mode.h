#pragma once

#include <stdint.h>

typedef enum : uint8_t {
    PLAYER_GAME_MODE_SURVIVAL,
    PLAYER_GAME_MODE_CREATIVE,
    PLAYER_GAME_MODE_SPECTATOR,
} PlayerGameMode;
