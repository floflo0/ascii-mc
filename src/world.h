#pragma once

#ifndef __wasm__
#include <pthread.h>
#endif

#include "block.h"
#include "camera.h"
#include "config.h"
#include "mesh.h"
#include "viewport.h"

typedef struct {
    int x, z;
    Mesh *mesh;
#ifndef __wasm__
    pthread_mutex_t mesh_mutex;
#endif
    bool loaded_by[4];
    Block blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];  // blocks[x][y][z]
} Chunk;

typedef struct {
    Chunk *chunks[WORLD_SIZE][WORLD_SIZE];  // chunks[x][z]
    uint32_t seed;
    BlockType place_block;
} World;

[[gnu::returns_nonnull]]
World *world_create(const uint32_t seed);

[[gnu::nonnull]]
void world_destroy(World *const self);

[[gnu::nonnull]]
void world_render(const World *const restrict self,
                  const Camera *const restrict camera,
                  const Viewport *const restrict viewport);

[[gnu::nonnull(1)]]
Chunk *world_get_chunk(const World *const self, const int x, const int z);

[[gnu::nonnull(1)]]
Block *world_get_block(const World *const self, const v3i block_position);

[[gnu::nonnull(1)]]
bool world_block_is_solid(const World *const self, const v3i block_position);

v2i world_position_to_chunk_coordinate(const v3f position);

[[gnu::nonnull(1)]]
void world_update_loaded_chunks(World *const self, const v2i old_chunk_position,
                                const v2i new_chunk_position,
                                const int8_t player_index);

[[gnu::nonnull(1)]]
void world_load_chunks_around_player(World *const self,
                                     const v2i player_chunk_position,
                                     const int8_t player_index);

[[gnu::nonnull]]
void world_place_block(World *const self, const v3i block_position);

[[gnu::nonnull]]
void world_break_block(World *const self, const v3i block_position);
