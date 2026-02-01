#pragma once

#include "camera_defs.h"
#include "gamepad_defs.h"
#include "player_defs.h"
#include "vec_defs.h"
#include "world.h"

[[gnu::nonnull(1, 5)]]
void player_init(Player *const restrict self, const int8_t player_index,
                 const uint8_t number_players, Gamepad *const restrict gamepad,
                 World *const restrict world, const float character_ratio);

[[gnu::nonnull]]
void player_destroy(Player *const self);

[[gnu::nonnull(1, 2)]]
void player_update(Player *const restrict self, World *const restrict world,
                   const float delta_time_seconds);

[[gnu::nonnull]]
void player_render(const Player *const restrict self,
                   const Camera *const restrict camera,
                   const Viewport *const restrict viewport);

[[gnu::nonnull(1)]]
void player_rotate(Player *const self, const v2f rotation);

[[gnu::nonnull]]
void player_jump(Player *const self);

[[gnu::nonnull(1, 3)]]
void player_render_ui(const Player *const restrict self,
                      const uint8_t number_players,
                      const World *const restrict world);

[[gnu::nonnull(1)]]
void player_update_viewport(Player *const self, const uint8_t number_players,
                            const float character_ratio);

[[gnu::nonnull]]
bool player_get_targeted_block(const Player *const restrict self,
                               const World *const restrict world,
                               v3i *const restrict block_position,
                               CollisionAxis *const restrict collision_axis);

[[gnu::nonnull(1, 2, 4)]]
void player_place_block(const Player *const restrict self,
                        const Player *const restrict players,
                        const uint8_t number_players,
                        World *const restrict world);

[[gnu::nonnull]]
void player_break_block(const Player *const restrict self,
                        World *const restrict world);

[[gnu::nonnull(1, 3)]]
void player_teleport(Player *const restrict self, v3i position,
                     World *const restrict world);

[[gnu::nonnull]]
void player_set_game_mode(Player *const self, const PlayerGameMode game_mode);
