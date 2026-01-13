#include "player.h"

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camera.h"
#include "config.h"
#include "controller.h"
#include "log.h"
#include "mesh.h"
#include "triangle_index_array.h"
#include "v3f_array.h"
#include "vec.h"
#include "window.h"

#define WORLD_ORIGIN (WORLD_SIZE / 2)

[[gnu::nonnull]]
static inline void player_generate_mesh(Player *const self) {
    assert(self != NULL);

    static const Color player_colors[4] = {
        PLAYER_0_COLOR,
        PLAYER_1_COLOR,
        PLAYER_2_COLOR,
        PLAYER_3_COLOR,
    };

    const float mesh_half_width = sqrtf(2.0f) * PLAYER_WIDTH / 4.0f;

    size_t i;

    i = v3f_array_grow(&self->mesh.vertices);
    self->mesh.vertices.array[i].x = -mesh_half_width;
    self->mesh.vertices.array[i].y = 0.0f;
    self->mesh.vertices.array[i].z = -mesh_half_width;

    i = v3f_array_grow(&self->mesh.vertices);
    self->mesh.vertices.array[i].x = -mesh_half_width;
    self->mesh.vertices.array[i].y = PLAYER_HEIGHT;
    self->mesh.vertices.array[i].z = -mesh_half_width;

    i = v3f_array_grow(&self->mesh.vertices);
    self->mesh.vertices.array[i].x = mesh_half_width;
    self->mesh.vertices.array[i].y = PLAYER_HEIGHT;
    self->mesh.vertices.array[i].z = -mesh_half_width;

    i = v3f_array_grow(&self->mesh.vertices);
    self->mesh.vertices.array[i].x = mesh_half_width;
    self->mesh.vertices.array[i].y = 0.0f;
    self->mesh.vertices.array[i].z = -mesh_half_width;

    i = v3f_array_grow(&self->mesh.vertices);
    self->mesh.vertices.array[i].x = -mesh_half_width;
    self->mesh.vertices.array[i].y = 0.0f;
    self->mesh.vertices.array[i].z = mesh_half_width;

    i = v3f_array_grow(&self->mesh.vertices);
    self->mesh.vertices.array[i].x = -mesh_half_width;
    self->mesh.vertices.array[i].y = PLAYER_HEIGHT;
    self->mesh.vertices.array[i].z = mesh_half_width;

    i = v3f_array_grow(&self->mesh.vertices);
    self->mesh.vertices.array[i].x = mesh_half_width;
    self->mesh.vertices.array[i].y = PLAYER_HEIGHT;
    self->mesh.vertices.array[i].z = mesh_half_width;

    i = v3f_array_grow(&self->mesh.vertices);
    self->mesh.vertices.array[i].x = mesh_half_width;
    self->mesh.vertices.array[i].y = 0.0f;
    self->mesh.vertices.array[i].z = mesh_half_width;

    // front face
    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 0;
    self->mesh.triangles.array[i].v2 = 1;
    self->mesh.triangles.array[i].v3 = 2;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V1_V2_FAR |
        TRIANGLE_EDGE_V2_V3_FAR;

    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 0;
    self->mesh.triangles.array[i].v2 = 2;
    self->mesh.triangles.array[i].v3 = 3;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V2_V3_FAR |
        TRIANGLE_EDGE_V3_V1_FAR;

    // left face
    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 3;
    self->mesh.triangles.array[i].v2 = 2;
    self->mesh.triangles.array[i].v3 = 6;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V1_V2_FAR |
        TRIANGLE_EDGE_V2_V3_FAR;

    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 3;
    self->mesh.triangles.array[i].v2 = 6;
    self->mesh.triangles.array[i].v3 = 7;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V2_V3_FAR |
        TRIANGLE_EDGE_V3_V1_FAR;

    // back face
    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 7;
    self->mesh.triangles.array[i].v2 = 6;
    self->mesh.triangles.array[i].v3 = 5;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V1_V2_FAR |
        TRIANGLE_EDGE_V2_V3_FAR;

    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 7;
    self->mesh.triangles.array[i].v2 = 5;
    self->mesh.triangles.array[i].v3 = 4;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V2_V3_FAR |
        TRIANGLE_EDGE_V3_V1_FAR;

    // right face
    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 4;
    self->mesh.triangles.array[i].v2 = 5;
    self->mesh.triangles.array[i].v3 = 1;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V1_V2_FAR |
        TRIANGLE_EDGE_V2_V3_FAR;

    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 4;
    self->mesh.triangles.array[i].v2 = 1;
    self->mesh.triangles.array[i].v3 = 0;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V2_V3_FAR |
        TRIANGLE_EDGE_V3_V1_FAR;

    // top face
    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 1;
    self->mesh.triangles.array[i].v2 = 5;
    self->mesh.triangles.array[i].v3 = 6;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V1_V2_FAR |
        TRIANGLE_EDGE_V2_V3_FAR;

    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 1;
    self->mesh.triangles.array[i].v2 = 6;
    self->mesh.triangles.array[i].v3 = 2;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V2_V3_FAR |
        TRIANGLE_EDGE_V3_V1_FAR;

    // bottom face
    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 4;
    self->mesh.triangles.array[i].v2 = 0;
    self->mesh.triangles.array[i].v3 = 3;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V1_V2_FAR |
        TRIANGLE_EDGE_V2_V3_FAR;

    i = triangle_index_array_grow(&self->mesh.triangles);
    self->mesh.triangles.array[i].v1 = 4;
    self->mesh.triangles.array[i].v2 = 3;
    self->mesh.triangles.array[i].v3 = 7;
    self->mesh.triangles.array[i].texture = NULL;
    self->mesh.triangles.array[i].color = player_colors[self->player_index];
    self->mesh.triangles.array[i].edges =
        TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V2_V3_FAR |
        TRIANGLE_EDGE_V3_V1_FAR;
}

void player_init(Player *const restrict self, const int8_t player_index,
                 const uint8_t number_players,
                 Controller *const restrict controller,
                 World *const restrict world) {
    assert(self != NULL);
    assert(world != NULL);

    self->player_index = player_index;

    self->velocity.x = 0.0f;
    self->velocity.y = 0.0f;
    self->velocity.z = 0.0f;

    self->input_velocity.x = 0.0f;
    self->input_velocity.y = 0.0f;
    self->input_velocity.z = 0.0f;

    self->health = PLAYER_MAX_HEALTH;

    self->game_mode = PLAYER_DEFAULT_GAME_MODE;

    self->controller = controller;
    if (controller != NULL)
        controller_set_player_index(controller, self->player_index);

    self->last_grounded_time_microseconds = 0;
    self->can_jump = false;

    viewport_from_player_index(&self->viewport, player_index, number_players);

    camera_init(&self->camera,
                (v3f){
                    .x = self->position.x,
                    .y = self->position.y + PLAYER_HEIGHT,
                    .z = self->position.z,
                },
                PLAYER_DEFAULT_YAW, PLAYER_DEFAULT_PITCH,
                (float)self->viewport.width / self->viewport.height);

    mesh_init(&self->mesh, 8, 12);
    player_generate_mesh(self);

    self->position.x = PLAYER_START_X + 0.5f;
    self->position.z = PLAYER_START_Z + 0.5f;

    world_load_chunks_around_player(
        world, world_position_to_chunk_coordinate(self->position),
        player_index);

    for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
        if (world_block_is_solid(world,
                                 (v3i){PLAYER_START_X, y, PLAYER_START_Z})) {
            self->position.y = y + 1.0f;
            return;
        }
    }
    self->position.y = 0.0f;
}

void player_destroy(Player *const self) {
    assert(self != NULL);
    if (self->controller != NULL) {
        controller_set_player_index(self->controller, -1);
    }
    mesh_destroy(&self->mesh);
}

[[gnu::nonnull(1)]]
static void player_damage(Player *const self, const int damage) {
    assert(self != NULL);
    assert(damage > 0);

    log_debugf("player %d take %d damages", self->player_index, damage);

    if (self->health > damage) {
        self->health -= damage;
    } else {
        self->health = 0;
        // TODO: the player should die
    }
}

[[gnu::nonnull(1, 3)]]
static void player_move(Player *const restrict self, const v3f velocity,
                        const World *const restrict world) {
    assert(self != NULL);
    assert(world != NULL);

    if (velocity.x == 0.0f && velocity.y == 0.0f && velocity.z == 0.0f) {
        return;
    }

    if (self->game_mode == PLAYER_GAME_MODE_SPECTATOR) {
        self->position = v3f_add(self->position, velocity);
        return;
    }

    int min_x;
    int max_x;
    if (velocity.x >= 0.0f) {
        min_x = floorf(self->position.x - (PLAYER_WIDTH / 2.0f));
        max_x = floorf(self->position.x + (PLAYER_WIDTH / 2.0f) + velocity.x);
    } else {
        min_x = floorf(self->position.x - (PLAYER_WIDTH / 2.0f) + velocity.x);
        max_x = floorf(self->position.x + (PLAYER_WIDTH / 2.0f));
    }
    assert(min_x <= max_x);

    int min_y;
    int max_y;
    if (velocity.y >= 0.0f) {
        min_y = floorf(self->position.y);
        max_y = floorf(self->position.y + PLAYER_HEIGHT + velocity.y);
    } else {
        min_y = floorf(self->position.y + velocity.y);
        max_y = floorf(self->position.y + PLAYER_HEIGHT);
    }
    assert(min_y <= max_y);

    int min_z;
    int max_z;
    if (velocity.z >= 0.0f) {
        min_z = floorf(self->position.z - (PLAYER_WIDTH / 2.0f));
        max_z = floorf(self->position.z + (PLAYER_WIDTH / 2.0f) + velocity.z);
    } else {
        min_z = floorf(self->position.z - (PLAYER_WIDTH / 2.0f) + velocity.z);
        max_z = floorf(self->position.z + (PLAYER_WIDTH / 2.0f));
    }
    assert(min_z <= max_z);

    const int player_x = floorf(self->position.x);
    const int player_y = floorf(self->position.y);
    const int player_z = floorf(self->position.z);

    float min_collision_time = 1.0f;
    CollisionAxis detected_collision_axis = COLLISION_AXIS_X_PLUS;
    int min_collision_distance = INT_MAX;

    v3i position;
    for (position.x = min_x; position.x <= max_x; ++position.x) {
        for (position.y = min_y; position.y <= max_y; ++position.y) {
            for (position.z = min_z; position.z <= max_z; ++position.z) {
                if (!world_block_is_solid(world, position)) continue;

                const Aabb aabb = {
                    .position =
                        {
                            .x = position.x - PLAYER_WIDTH / 2.0f,
                            .y = position.y - PLAYER_HEIGHT,
                            .z = position.z - PLAYER_WIDTH / 2.0f,
                        },
                    .size =
                        {
                            .x = 1.0f + PLAYER_WIDTH,
                            .y = 1.0f + PLAYER_HEIGHT,
                            .z = 1.0f + PLAYER_WIDTH,
                        },
                };

                float collision_time;
                CollisionAxis collision_axis;
                if (!aabb_collide_ray(&aabb, self->position, velocity,
                                      &collision_time, &collision_axis)) {
                    continue;
                }

                const int collision_distance = abs(position.x - player_x) +
                                               abs(position.y - player_y) +
                                               abs(position.z - player_z);
                if ((collision_time == min_collision_time &&
                     collision_distance < min_collision_distance) ||
                    collision_time < min_collision_time) {
                    min_collision_time = collision_time;
                    detected_collision_axis = collision_axis;
                    min_collision_distance = collision_distance;
                }
            }
        }
    }

    self->position =
        v3f_add(self->position, v3f_mul(velocity, min_collision_time));

    if (min_collision_time != 1.0f) {
        v3f remaining_velocity = v3f_mul(velocity, 1.0f - min_collision_time);
        switch (detected_collision_axis) {
            case COLLISION_AXIS_X_PLUS:
            case COLLISION_AXIS_X_MINUS:
                remaining_velocity.x = 0.0f;
                break;

            case COLLISION_AXIS_Y_PLUS:
            case COLLISION_AXIS_Y_MINUS:
                remaining_velocity.y = 0.0f;
                if (self->game_mode != PLAYER_GAME_MODE_SURVIVAL) break;
                if (self->velocity.y < 0.0f) {
                    const float tmp = self->velocity.y / G_FORCE;
                    const int fall_height =
                        roundf((G_FORCE / 2.0f) * tmp * tmp);
                    if (fall_height > PLAYER_SAFE_FALL_HEIGHT) {
                        log_debugf("player %d fall damages",
                                   self->player_index);
                        const int damage =
                            fall_height - PLAYER_SAFE_FALL_HEIGHT;
                        player_damage(self, damage);
                    }
                }
                self->velocity.y = 0.0f;
                break;

            case COLLISION_AXIS_Z_PLUS:
            case COLLISION_AXIS_Z_MINUS:
                remaining_velocity.z = 0.0f;
                break;
        }
        player_move(self, remaining_velocity, world);
    }
}

[[gnu::nonnull]]
static inline bool player_is_grounded(const Player *const restrict self,
                                      const World *const restrict world) {
    assert(self != NULL);
    assert(world != NULL);

    if (self->velocity.y > 0.0f ||
        fabsf(self->position.y - ceilf(self->position.y)) >
            PLAYER_GROUNDED_EPSILON) {
        return false;
    }

    const float half_player_width = PLAYER_WIDTH * 0.5f;
    const int min_x = floorf(self->position.x - half_player_width);
    const int max_x = floorf(self->position.x + half_player_width);
    const int min_z = floorf(self->position.z - half_player_width);
    const int max_z = floorf(self->position.z + half_player_width);
    v3i position = {.x = min_x, .y = floorf(self->position.y) - 1};
    for (; position.x <= max_x; ++position.x) {
        for (position.z = min_z; position.z <= max_z; ++position.z) {
            if (world_block_is_solid(world, position)) return true;
        }
    }

    return false;
}

[[gnu::nonnull]]
static inline void player_update_mesh(Player *const self) {
    assert(self != NULL);

    mesh_clear(&self->mesh);
    player_generate_mesh(self);

    m4f rotation_y_matrix;
    m4f_rotation_y(rotation_y_matrix, self->camera.yaw);

    for (size_t i = 0; i < self->mesh.vertices.length; ++i) {
        self->mesh.vertices.array[i] =
            mul_m4f_v3f(rotation_y_matrix, self->mesh.vertices.array[i]).xyz;
        self->mesh.vertices.array[i] =
            v3f_add(self->mesh.vertices.array[i], self->position);
    }
}

void player_update(Player *const restrict self, World *const restrict world,
                   const float delta_time_seconds) {
    assert(self != NULL);
    assert(world != NULL);
    assert(delta_time_seconds >= 0.0f);

    if (self->game_mode == PLAYER_GAME_MODE_SURVIVAL) {
        self->velocity.y -= G_FORCE * delta_time_seconds * 0.5f;
    }

    const float cos_yaw = cosf(self->camera.yaw);
    const float sin_yaw = sinf(self->camera.yaw);

    const v3f velocity = {
        .x = self->velocity.x + self->input_velocity.x * cos_yaw +
             -self->input_velocity.z * sin_yaw,
        .y = self->velocity.y * delta_time_seconds + self->input_velocity.y,
        .z = self->velocity.z + self->input_velocity.x * sin_yaw +
             self->input_velocity.z * cos_yaw,
    };

    const v2i old_chunk_position =
        world_position_to_chunk_coordinate(self->position);

    player_move(self, velocity, world);

    const v2i new_chunk_position =
        world_position_to_chunk_coordinate(self->position);

    if (!v2i_equals(old_chunk_position, new_chunk_position)) {
        world_update_loaded_chunks(world, old_chunk_position,
                                   new_chunk_position, self->player_index);
    }

    player_update_mesh(self);

    if (self->game_mode == PLAYER_GAME_MODE_SURVIVAL) {
        self->velocity.y -= G_FORCE * delta_time_seconds * 0.5f;
    }

    if (player_is_grounded(self, world)) {
        self->last_grounded_time_microseconds = get_time_microseconds();
        self->can_jump = true;
    }

    self->camera.position.x = self->position.x;
    self->camera.position.y = self->position.y + PLAYER_CAMERA_HEIGHT;
    self->camera.position.z = self->position.z;

    self->is_targeting_a_block = player_get_targeted_block(
        self, world, &self->targeted_block, &self->targeted_block_face);

    self->input_velocity.x = 0.0f;
    self->input_velocity.y = 0.0f;
    self->input_velocity.z = 0.0f;
}

void player_render(const Player *const restrict self,
                   const Camera *const restrict camera,
                   const Viewport *const restrict viewport) {
    assert(self != NULL);
    assert(camera != NULL);
    assert(viewport != NULL);

    mesh_render(&self->mesh, camera, viewport);
}

inline void player_rotate(Player *const self, const v2f rotation) {
    assert(self != NULL);
    camera_rotate(&self->camera, rotation);
}

void player_jump(Player *const self) {
    assert(self != NULL);

    if (!self->can_jump ||
        get_time_microseconds() - self->last_grounded_time_microseconds >
            PLAYER_COYOTE_TIME_MICROSECONDS) {
        return;
    }

    log_debugf("player %d jump", self->player_index);
    self->velocity.y = PLAYER_JUMP_SPEED;
    self->can_jump = false;
}

[[gnu::nonnull(1)]]
static inline void player_render_informations(const Player *const self,
                                              const uint8_t number_players) {
    assert(self != NULL);
    assert(0 < number_players && number_players <= 4);

    char player_position_string[34];
    const int player_position_string_length =
        snprintf(player_position_string, sizeof(player_position_string),
                 "| %.1f %.1f %.1f |", self->position.x, self->position.y,
                 self->position.z);
    assert(0 < player_position_string_length);
    assert(player_position_string_length < (int)sizeof(player_position_string));

    char horizontal_border[player_position_string_length + 1];
    horizontal_border[0] = '+';
    memset(&horizontal_border[1], '-', player_position_string_length - 2);
    horizontal_border[player_position_string_length - 1] = '+';
    horizontal_border[player_position_string_length] = '\0';

    v2i position = {.x = self->viewport.x_offset + 2,
                    .y = self->viewport.y_offset + 1};

    window_render_string(position, horizontal_border, COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
    ++position.y;

    if (number_players > 1) {
        char player_name_string[sizeof(player_position_string)];
        snprintf(player_name_string, sizeof(player_name_string),
                 "| Player %-*d |", player_position_string_length - 11,
                 self->player_index);
        window_render_string(position, player_name_string, COLOR_WHITE,
                             WINDOW_Z_BUFFER_FRONT);
        ++position.y;
    }
    window_render_string(position, player_position_string, COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
    ++position.y;

    window_render_string(position, horizontal_border, COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
}

[[gnu::nonnull]]
static inline void player_render_crosshair(const Player *const self) {
    assert(self != NULL);
    assert(window.is_init);

    const int center_x = self->viewport.x_offset + self->viewport.width / 2;
    const int center_y = (self->viewport.y_offset + self->viewport.height / 2);

    window_set_pixel((center_y - 1) * window.width + center_x, '|',
                     PLAYER_CROSSHAIR_COLOR, WINDOW_Z_BUFFER_FRONT);
    const int center_index = center_y * window.width + center_x;
    for (uint8_t i = 1; i <= 3; ++i) {
        window_set_pixel(center_index - i, '-', PLAYER_CROSSHAIR_COLOR,
                         WINDOW_Z_BUFFER_FRONT);
        window_set_pixel(center_index + i, '-', PLAYER_CROSSHAIR_COLOR,
                         WINDOW_Z_BUFFER_FRONT);
    }
    window_set_pixel(center_index, '+', PLAYER_CROSSHAIR_COLOR,
                     WINDOW_Z_BUFFER_FRONT);
    window_set_pixel((center_y + 1) * window.width + center_x, '|',
                     PLAYER_CROSSHAIR_COLOR, WINDOW_Z_BUFFER_FRONT);
}

[[gnu::nonnull]]
static inline void player_render_targeted_block(
    const Player *const restrict self, const World *const restrict world) {
    assert(self != NULL);
    assert(world != NULL);

    if (!self->is_targeting_a_block) return;

    const char *block_name =
        block_get_name(world_get_block(world, self->targeted_block)->type);
    const size_t block_name_length = strlen(block_name);

    char horizontal_border[block_name_length + 5];
    memset(&horizontal_border[1], '-', block_name_length + 2);
    horizontal_border[0] = '+';
    horizontal_border[block_name_length + 3] = '+';
    horizontal_border[block_name_length + 4] = '\0';
    v2i position = {
        .x = self->viewport.x_offset + self->viewport.width / 2 -
             block_name_length / 2 - 2,
        .y = self->viewport.y_offset + self->viewport.height - 4,
    };
    window_render_string(position, horizontal_border, COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
    ++position.y;
    char block_name_buffer[block_name_length + 5];
    snprintf(block_name_buffer, sizeof(block_name_buffer), "| %s |",
             block_name);
    window_render_string(position, block_name_buffer, COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
    ++position.y;
    window_render_string(position, horizontal_border, COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
}

[[gnu::nonnull]]
static inline void player_render_health(const Player *const self) {
    assert(self != NULL);
    assert(self->health <= PLAYER_MAX_HEALTH);
    static_assert(PLAYER_MAX_HEALTH < 100,
                  "PLAYER_MAX_HEALTH should have at most 2 digits");

    v2i position = {
        .x = self->viewport.x_offset + 2,
        .y = self->viewport.y_offset + self->viewport.height - 4,
    };
    window_render_string(position, "+---------------------------+", COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
    ++position.y;

    window_render_string(position, "| [", COLOR_WHITE, WINDOW_Z_BUFFER_FRONT);
    char health_bar[PLAYER_MAX_HEALTH + 1];
    uint8_t i = 0;
    for (; i < self->health; ++i) {
        health_bar[i] = '#';
    }
    for (; i < PLAYER_MAX_HEALTH; ++i) {
        health_bar[i] = ' ';
    }
    health_bar[PLAYER_MAX_HEALTH] = '\0';
    window_render_string((v2i){position.x + 3, position.y}, health_bar,
                         COLOR_RED, WINDOW_Z_BUFFER_FRONT);

    char health_number[8];
    snprintf(health_number, sizeof(health_number), "] %2u |", self->health);
    window_render_string((v2i){position.x + 3 + PLAYER_MAX_HEALTH, position.y},
                         health_number, COLOR_WHITE, WINDOW_Z_BUFFER_FRONT);
    ++position.y;

    window_render_string(position, "+---------------------------+", COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
}

void player_render_ui(const Player *const restrict self,
                      const uint8_t number_players,
                      const World *const restrict world) {
    assert(self != NULL);
    assert(0 < number_players && number_players <= 4);
    assert(world != NULL);

    player_render_informations(self, number_players);
    player_render_crosshair(self);

    if (self->game_mode != PLAYER_GAME_MODE_SPECTATOR) {
        player_render_targeted_block(self, world);

        if (self->game_mode == PLAYER_GAME_MODE_SURVIVAL) {
            player_render_health(self);
        }
    }
}

void player_update_viewport(Player *const self, const uint8_t number_players) {
    assert(self != NULL);
    assert(0 < number_players && number_players <= 4);

    viewport_from_player_index(&self->viewport, self->player_index,
                               number_players);
    camera_set_aspect_ratio(
        &self->camera, (float)self->viewport.width / self->viewport.height);
}

bool player_get_targeted_block(const Player *const restrict self,
                               const World *const restrict world,
                               v3i *const restrict block_position,
                               CollisionAxis *const restrict collision_axis) {
    assert(self != NULL);
    assert(world != NULL);
    assert(block_position != NULL);
    assert(collision_axis != NULL);

    const v3f ray_start = self->camera.position;
    const v3f ray_direction =
        v3f_mul(camera_get_look_direction(&self->camera), PLAYER_RANGE);

    int min_x = floorf(ray_start.x);
    int max_x = floorf(ray_start.x + ray_direction.x);
    if (min_x > max_x) {
        int tmp = min_x;
        min_x = max_x;
        max_x = tmp;
    }

    int min_y = floorf(ray_start.y);
    int max_y = floorf(ray_start.y + ray_direction.y);
    if (min_y > max_y) {
        int tmp = min_y;
        min_y = max_y;
        max_y = tmp;
    }

    int min_z = floorf(ray_start.z);
    int max_z = floorf(ray_start.z + ray_direction.z);
    if (min_z > max_z) {
        int tmp = min_z;
        min_z = max_z;
        max_z = tmp;
    }

    Aabb aabb = {.size = {1.0f, 1.0f, 1.0f}};

    float min_collision_time = FLT_MAX;
    v3i position;
    for (position.x = min_x; position.x <= max_x; ++position.x) {
        for (position.y = min_y; position.y <= max_y; ++position.y) {
            for (position.z = min_z; position.z <= max_z; ++position.z) {
                if (!world_block_is_solid(world, position)) continue;

                aabb.position.x = position.x;
                aabb.position.y = position.y;
                aabb.position.z = position.z;

                float collision_time;
                CollisionAxis detected_collision_axis;
                if (!aabb_collide_ray(&aabb, ray_start, ray_direction,
                                      &collision_time,
                                      &detected_collision_axis)) {
                    continue;
                }

                if (collision_time >= min_collision_time) continue;

                min_collision_time = collision_time;
                *collision_axis = detected_collision_axis;
                *block_position = position;
            }
        }
    }

    return min_collision_time <= 1.0f;
}

[[gnu::nonnull(1)]]
static inline bool player_collide_block(const Player *const self,
                                        const v3i block_position) {
    const float player_min_x = self->position.x - PLAYER_WIDTH / 2.0f;
    const float player_min_y = self->position.y;
    const float player_min_z = self->position.z - PLAYER_WIDTH / 2.0f;
    const float player_max_x = self->position.x + PLAYER_WIDTH / 2.0f;
    const float player_max_y = self->position.y + PLAYER_HEIGHT;
    const float player_max_z = self->position.z + PLAYER_WIDTH / 2.0f;

    return player_min_x < block_position.x + 1 &&
           block_position.x < player_max_x &&
           player_min_y < block_position.y + 1 &&
           block_position.y < player_max_y &&
           player_min_z < block_position.z + 1 &&
           block_position.z < player_max_z;
}

void player_place_block(const Player *const restrict self,
                        const Player *const restrict players,
                        const uint8_t number_players, World *restrict world) {
    assert(self != NULL);
    assert(players != NULL);
    assert(0 < number_players && number_players <= 4);
    assert(world != NULL);

    if (self->game_mode == PLAYER_GAME_MODE_SPECTATOR) return;

    if (!self->is_targeting_a_block) return;

    v3i block_position = self->targeted_block;

    switch (self->targeted_block_face) {
        case COLLISION_AXIS_X_PLUS:
            ++block_position.x;
            break;

        case COLLISION_AXIS_X_MINUS:
            --block_position.x;
            break;

        case COLLISION_AXIS_Y_PLUS:
            ++block_position.y;
            break;

        case COLLISION_AXIS_Y_MINUS:
            --block_position.y;
            break;

        case COLLISION_AXIS_Z_PLUS:
            ++block_position.z;
            break;

        case COLLISION_AXIS_Z_MINUS:
            --block_position.z;
            break;
    }

    if (block_position.y < 0 || block_position.y >= CHUNK_HEIGHT) return;

    for (uint8_t i = 0; i < number_players; ++i) {
        if (players[i].game_mode == PLAYER_GAME_MODE_SPECTATOR) continue;

        if (player_collide_block(&players[i], block_position)) return;
    }

    world_place_block(world, block_position);
}

void player_break_block(const Player *const restrict self,
                        World *const restrict world) {
    assert(self != NULL);
    assert(world != NULL);

    if (self->game_mode == PLAYER_GAME_MODE_SPECTATOR) return;

    if (!self->is_targeting_a_block) return;

    log_debugf("player %d try to break block (%d, %d, %d)", self->player_index,
               self->targeted_block.x, self->targeted_block.y,
               self->targeted_block.z);
    world_break_block(world, self->targeted_block);
}

void player_teleport(Player *const restrict self, v3i position,
                     World *const restrict world) {
    assert(self != NULL);
    assert(world != NULL);

    const v2i old_chunk_position =
        world_position_to_chunk_coordinate(self->position);

    self->position.x = position.x + 0.5f;
    self->position.z = position.z + 0.5f;

    const v2i new_chunk_position =
        world_position_to_chunk_coordinate(self->position);

    if (!v2i_equals(old_chunk_position, new_chunk_position)) {
        world_update_loaded_chunks(world, old_chunk_position,
                                   new_chunk_position, self->player_index);
    }

    if (position.y == -1) {
        for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
            if (world_block_is_solid(world, (v3i){position.x, y, position.z})) {
                self->position.y = y + 1.0f;
                goto exit_loop;
            }
        }
        self->position.y = 0.0f;
    exit_loop:
    } else if (self->game_mode != PLAYER_GAME_MODE_SPECTATOR) {
        while (true) {
            if (world_block_is_solid(world, position)) {
                ++position.y;
                continue;
                ;
            }

            if (world_block_is_solid(
                    world, (v3i){position.x, position.y + 1, position.z})) {
                position.y += 2;
                continue;
            }

            break;
        }
        self->position.y = position.y;
    }

    log_debugf("teleporting player to x=%.2f y=%.2f z=%.2f", self->position.x,
               self->position.y, self->position.z);
}

void player_set_game_mode(Player *const self, const PlayerGameMode game_mode) {
    assert(self != NULL);

    if (self->game_mode == game_mode) return;

    log_debugf("set player %u game mode to %u", self->player_index, game_mode);
    self->game_mode = game_mode;

    if (game_mode == PLAYER_GAME_MODE_SPECTATOR) {
        self->is_targeting_a_block = false;
    } else {
        // TODO: check for collision
    }

    if (game_mode != PLAYER_GAME_MODE_SURVIVAL) {
        self->velocity.x = 0.0f;
        self->velocity.y = 0.0f;
        self->velocity.z = 0.0f;
    }
}
