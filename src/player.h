#pragma once

#include "camera.h"
#include "collision.h"
#include "controller_defs.h"
#include "mesh.h"
#include "player_game_mode.h"
#include "vec.h"
#include "viewport.h"
#include "world.h"

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

typedef struct {
    Camera *camera;
    Controller *controller;
    Mesh *mesh;
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

void player_init(Player *const restrict self, const int8_t player_index,
                 const uint8_t number_players,
                 Controller *const restrict controller,
                 World *const restrict world) NONNULL(1, 5);

void player_destroy(Player *const self) NONNULL();

void player_update(Player *const restrict self, World *const restrict world,
                   const float delta_time_seconds) NONNULL(1, 2);

void player_render(const Player *const restrict self,
                   const Camera *const restrict camera,
                   const Viewport *const restrict viewport) NONNULL();

void player_rotate(Player *const self, const v2f rotation) NONNULL(1);

void player_jump(Player *const self) NONNULL();

void player_render_ui(const Player *const restrict self,
                      const uint8_t number_players,
                      const World *const restrict world) NONNULL(1, 3);

void player_update_viewport(Player *const self, const uint8_t number_players)
    NONNULL(1);

bool player_get_targeted_block(const Player *const restrict self,
                               const World *const restrict world,
                               v3i *const restrict block_position,
                               CollisionAxis *const restrict collision_axis)
    NONNULL();

void player_place_block(const Player *const restrict self,
                        const Player *const restrict players,
                        const uint8_t number_players,
                        World *const restrict world) NONNULL(1, 2, 4);

void player_break_block(const Player *const restrict self,
                        World *const restrict world) NONNULL();

void player_teleport(Player *const restrict self, v3i position,
                     World *const restrict world) NONNULL(1, 3);

void player_set_game_mode(Player *const self, const PlayerGameMode game_mode)
    NONNULL();
