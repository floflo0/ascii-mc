#pragma once

#include "camera_defs.h"
#include "collision_defs.h"
#include "gamepad_defs.h"
#include "mesh_defs.h"
#include "player_game_mode_defs.h"
#include "viewport_defs.h"

typedef struct {
    Camera camera;
    Gamepad *gamepad;
    Mesh mesh;
    uint64_t last_grounded_time_microseconds;
    v3f position;
    v3f velocity;
    v3f input_velocity;
    Viewport viewport;
    v3i targeted_block;
    CollisionAxis targeted_block_face;
    bool is_targeting_a_block;
    uint8_t health;
    PlayerGameMode game_mode;
    int8_t player_index;
    bool can_jump;
} Player;
