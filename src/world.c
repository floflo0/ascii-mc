#include "world.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// #define LOG_LEVEL_ERROR
#include "log.h"
#include "perlin_noise.h"
#include "textures.h"
#include "threads.h"
#include "utils.h"

#define WORLD_ORIGIN (WORLD_SIZE / 2)

#define vertex_indices_index(x, y, z)                                         \
    ((x) * ((CHUNK_HEIGHT + 1) * (CHUNK_SIZE + 1)) + (y) * (CHUNK_SIZE + 1) + \
     (z))

[[gnu::nonnull(1, 2, 3)]]
static int chunk_get_vertex_index(const Chunk *const restrict self,
                                  const Mesh *const restrict mesh,
                                  int *vertex_indices, const uint8_t x,
                                  const uint8_t y, const uint8_t z) {
    assert(self != NULL);
    assert(mesh != NULL);
    assert(vertex_indices != NULL);

    if (vertex_indices[vertex_indices_index(x, y, z)] != -1)
        return vertex_indices[vertex_indices_index(x, y, z)];

    const size_t i = v3f_array_grow(mesh->vertices);
    mesh->vertices->array[i].x = self->x * CHUNK_SIZE + x;
    mesh->vertices->array[i].y = y;
    mesh->vertices->array[i].z = self->z * CHUNK_SIZE + z;
    vertex_indices[vertex_indices_index(x, y, z)] = i;
    return i;
}

[[gnu::nonnull]]
static inline Mesh *chunk_generate_mesh(const Chunk *const restrict self,
                                        const World *const restrict world) {
    assert(self != NULL);
    assert(world != NULL);

    static const Color block_top_colors[] = {
#define BLOCK(name, name_string, top_color, ...) top_color,
        BLOCKS
#undef BLOCK
    };

    static const Color block_side_colors[] = {
#define BLOCK(name, name_string, top_color, side_color, ...) side_color,
        BLOCKS
#undef BLOCK
    };

    static const Color block_bottom_colors[] = {
#define BLOCK(name, name_string, top_color, side_color, bottom_color, ...) \
    bottom_color,
        BLOCKS
#undef BLOCK
    };

    static const Texture *const block_top_textures[] = {
#define BLOCK(name, name_string, top_color, _side_color, _bottom_color, \
              top_texture, ...)                                         \
    top_texture,
        BLOCKS
#undef BLOCK
    };

    static const Texture *const block_side_textures[] = {
#define BLOCK(name, name_string, top_color, _side_color, _bottom_color, \
              top_texture, side_texture, ...)                           \
    side_texture,
        BLOCKS
#undef BLOCK
    };

    static const Texture *const block_bottom_textures[] = {
#define BLOCK(name, name_string, top_color, _side_color, _bottom_color, \
              top_texture, side_texture, bottom_texture)                \
    bottom_texture,
        BLOCKS
#undef BLOCK
    };

#if !defined(PROD) && !defined(LOG_LEVEL_ERROR)
    const uint64_t start = get_time_microseconds();
#endif

    Mesh *const mesh = mesh_create(1024, 1024);

    int vertex_indices[(CHUNK_SIZE + 1) * (CHUNK_HEIGHT + 1) *
                       (CHUNK_SIZE + 1)];

    for (int x = 0; x <= CHUNK_SIZE; ++x) {
        for (int y = 0; y <= CHUNK_HEIGHT; ++y) {
            for (int z = 0; z <= CHUNK_SIZE; ++z) {
                vertex_indices[vertex_indices_index(x, y, z)] = -1;
            }
        }
    }

    const Chunk *const front_chunk =
        world_get_chunk(world, self->x, self->z - 1);
    const Chunk *const back_chunk =
        world_get_chunk(world, self->x, self->z + 1);
    const Chunk *const left_chunk =
        world_get_chunk(world, self->x - 1, self->z);
    const Chunk *const right_chunk =
        world_get_chunk(world, self->x + 1, self->z);

    const Chunk *const front_left_chunk =
        world_get_chunk(world, self->x - 1, self->z - 1);
    const Chunk *const front_right_chunk =
        world_get_chunk(world, self->x + 1, self->z - 1);
    const Chunk *const back_left_chunk =
        world_get_chunk(world, self->x - 1, self->z + 1);
    const Chunk *const back_right_chunk =
        world_get_chunk(world, self->x + 1, self->z + 1);

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                if (self->blocks[x][y][z].type == BLOCK_TYPE_AIR) continue;

                const Color side_color =
                    block_side_colors[self->blocks[x][y][z].type];
                const Texture *const side_texture =
                    block_side_textures[self->blocks[x][y][z].type];

                const bool is_front_face_visible =
                    (z == 0 && front_chunk != NULL &&
                     !front_chunk->blocks[x][y][CHUNK_SIZE - 1].type) ||
                    (z != 0 && !self->blocks[x][y][z - 1].type);

                const bool is_back_face_visible =
                    (z == CHUNK_SIZE - 1 && back_chunk != NULL &&
                     !back_chunk->blocks[x][y][0].type) ||
                    (z != CHUNK_SIZE - 1 && !self->blocks[x][y][z + 1].type);

                const bool is_left_face_visible =
                    (x == 0 && left_chunk != NULL &&
                     !left_chunk->blocks[CHUNK_SIZE - 1][y][z].type) ||
                    (x != 0 && !self->blocks[x - 1][y][z].type);

                const bool is_right_face_visible =
                    (x == CHUNK_SIZE - 1 && right_chunk != NULL &&
                     !right_chunk->blocks[0][y][z].type) ||
                    (x != CHUNK_SIZE - 1 && !self->blocks[x + 1][y][z].type);

                const bool block_front_left =
                    (z == 0 && x == 0 && front_left_chunk != NULL &&
                     front_left_chunk->blocks[CHUNK_SIZE - 1][y][CHUNK_SIZE - 1]
                         .type) ||
                    (z == 0 && x != 0 && front_chunk != NULL &&
                     front_chunk->blocks[x - 1][y][CHUNK_SIZE - 1].type) ||
                    (z != 0 && x == 0 && left_chunk != NULL &&
                     left_chunk->blocks[CHUNK_SIZE - 1][y][z - 1].type) ||
                    (z != 0 && x != 0 && self->blocks[x - 1][y][z - 1].type);

                const bool block_front_right =
                    (z == 0 && x == CHUNK_SIZE - 1 &&
                     front_right_chunk != NULL &&
                     front_right_chunk->blocks[0][y][CHUNK_SIZE - 1].type) ||
                    (z == 0 && x != CHUNK_SIZE - 1 && front_chunk != NULL &&
                     front_chunk->blocks[x + 1][y][CHUNK_SIZE - 1].type) ||
                    (z != 0 && x == CHUNK_SIZE - 1 && right_chunk != NULL &&
                     right_chunk->blocks[0][y][z - 1].type) ||
                    (z != 0 && x != CHUNK_SIZE - 1 &&
                     self->blocks[x + 1][y][z - 1].type);

                const bool block_back_left =
                    (z == CHUNK_SIZE - 1 && x == 0 && back_left_chunk != NULL &&
                     back_left_chunk->blocks[CHUNK_SIZE - 1][y][0].type) ||
                    (z == CHUNK_SIZE - 1 && x != 0 && back_chunk != NULL &&
                     back_chunk->blocks[x - 1][y][0].type) ||
                    (z != CHUNK_SIZE - 1 && x == 0 && left_chunk != NULL &&
                     left_chunk->blocks[CHUNK_SIZE - 1][y][z + 1].type) ||
                    (z != CHUNK_SIZE - 1 && x != 0 &&
                     self->blocks[x - 1][y][z + 1].type);

                const bool block_back_right =
                    (z == CHUNK_SIZE - 1 && x == CHUNK_SIZE - 1 &&
                     back_right_chunk != NULL &&
                     back_right_chunk->blocks[0][y][0].type) ||
                    (z == CHUNK_SIZE - 1 && x != CHUNK_SIZE - 1 &&
                     back_chunk != NULL &&
                     back_chunk->blocks[x + 1][y][0].type) ||
                    (z != CHUNK_SIZE - 1 && x == CHUNK_SIZE - 1 &&
                     right_chunk != NULL &&
                     right_chunk->blocks[0][y][z + 1].type) ||
                    (z != CHUNK_SIZE - 1 && x != CHUNK_SIZE - 1 &&
                     self->blocks[x + 1][y][z + 1].type);

                const bool block_top_front =
                    y != CHUNK_HEIGHT - 1 &&
                    ((z == 0 && front_chunk != NULL &&
                      front_chunk->blocks[x][y + 1][CHUNK_SIZE - 1].type) ||
                     (z != 0 && self->blocks[x][y + 1][z - 1].type));

                const bool block_top_back =
                    y != CHUNK_HEIGHT - 1 &&
                    ((z == CHUNK_SIZE - 1 && back_chunk != NULL &&
                      back_chunk->blocks[x][y + 1][0].type) ||
                     (z != CHUNK_SIZE - 1 &&
                      self->blocks[x][y + 1][z + 1].type));

                const bool block_top_left =
                    y != CHUNK_HEIGHT - 1 &&
                    ((x == 0 && left_chunk != NULL &&
                      left_chunk->blocks[CHUNK_SIZE - 1][y + 1][z].type) ||
                     (x != 0 && self->blocks[x - 1][y + 1][z].type));

                const bool block_top_right =
                    y != CHUNK_HEIGHT - 1 &&
                    ((x == CHUNK_SIZE - 1 && right_chunk != NULL &&
                      right_chunk->blocks[0][y + 1][z].type) ||
                     (x != CHUNK_SIZE - 1 &&
                      self->blocks[x + 1][y + 1][z].type));

                const bool block_bottom_front =
                    y != 0 &&
                    ((z == 0 && front_chunk != NULL &&
                      front_chunk->blocks[x][y - 1][CHUNK_SIZE - 1].type) ||
                     (z != 0 && self->blocks[x][y - 1][z - 1].type));

                const bool block_bottom_back =
                    y != 0 && ((z == CHUNK_SIZE - 1 && back_chunk != NULL &&
                                back_chunk->blocks[x][y - 1][0].type) ||
                               (z != CHUNK_SIZE - 1 &&
                                self->blocks[x][y - 1][z + 1].type));

                const bool block_bottom_left =
                    y != 0 &&
                    ((x == 0 && left_chunk != NULL &&
                      left_chunk->blocks[CHUNK_SIZE - 1][y - 1][z].type) ||
                     (x != 0 && self->blocks[x - 1][y - 1][z].type));

                const bool block_bottom_right =
                    y != 0 && ((x == CHUNK_SIZE - 1 && right_chunk != NULL &&
                                right_chunk->blocks[0][y - 1][z].type) ||
                               (x != CHUNK_SIZE - 1 &&
                                self->blocks[x + 1][y - 1][z].type));

                size_t i;
                TriangleIndex *triangle;
                if (is_front_face_visible) {
                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y, z);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y + 1, z);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y + 1, z);
                    triangle->uv1 = (v2f){0.0f, 1.0f};
                    triangle->uv2 = (v2f){0.0f, 0.0f};
                    triangle->uv3 = (v2f){1.0f, 0.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = side_texture;
                    triangle->color = side_color;

                    if (is_left_face_visible || block_front_left) {
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;
                    }

                    if ((y == CHUNK_HEIGHT - 1 ||
                         !self->blocks[x][y + 1][z].type) ||
                        block_top_front)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;

                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y + 1, z);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y, z);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y, z);
                    triangle->uv1 = (v2f){1.0f, 0.0f};
                    triangle->uv2 = (v2f){1.0f, 1.0f};
                    triangle->uv3 = (v2f){0.0f, 1.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = side_texture;
                    triangle->color = side_color;

                    if (is_right_face_visible || block_front_right)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if ((y == 0 || !self->blocks[x][y - 1][z].type) ||
                        block_bottom_front)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;
                }

                if (is_right_face_visible) {
                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y, z);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y + 1, z);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y + 1, z + 1);
                    triangle->uv1 = (v2f){0.0f, 1.0f};
                    triangle->uv2 = (v2f){0.0f, 0.0f};
                    triangle->uv3 = (v2f){1.0f, 0.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = side_texture;
                    triangle->color = side_color;

                    if (is_front_face_visible || block_front_right)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if ((y == CHUNK_HEIGHT - 1 ||
                         !self->blocks[x][y + 1][z].type) ||
                        block_top_right)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;

                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y + 1, z + 1);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y, z + 1);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y, z);
                    triangle->uv1 = (v2f){1.0f, 0.0f};
                    triangle->uv2 = (v2f){1.0f, 1.0f};
                    triangle->uv3 = (v2f){0.0f, 1.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = side_texture;
                    triangle->color = side_color;

                    if (is_back_face_visible || block_back_right)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if ((y == 0 || !self->blocks[x][y - 1][z].type) ||
                        block_bottom_right)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;
                }

                if (is_back_face_visible) {
                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y, z + 1);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y + 1, z + 1);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y + 1, z + 1);
                    triangle->uv1 = (v2f){0.0f, 1.0f};
                    triangle->uv2 = (v2f){0.0f, 0.0f};
                    triangle->uv3 = (v2f){1.0f, 0.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = side_texture;
                    triangle->color = side_color;

                    if (is_right_face_visible || block_back_right)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if ((y == CHUNK_HEIGHT - 1 ||
                         !self->blocks[x][y + 1][z].type) ||
                        block_top_back)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;

                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y + 1, z + 1);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y, z + 1);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y, z + 1);
                    triangle->uv1 = (v2f){1.0f, 0.0f};
                    triangle->uv2 = (v2f){1.0f, 1.0f};
                    triangle->uv3 = (v2f){0.0f, 1.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = side_texture;
                    triangle->color = side_color;

                    if (is_left_face_visible || block_back_left)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if ((y == 0 || !self->blocks[x][y - 1][z].type) ||
                        block_bottom_back)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;
                }

                if (is_left_face_visible) {
                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y, z + 1);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y + 1, z + 1);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y + 1, z);
                    triangle->uv1 = (v2f){0.0f, 1.0f};
                    triangle->uv2 = (v2f){0.0f, 0.0f};
                    triangle->uv3 = (v2f){1.0f, 0.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = side_texture;
                    triangle->color = side_color;

                    if (is_back_face_visible || block_back_left)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if ((y == CHUNK_HEIGHT - 1 ||
                         !self->blocks[x][y + 1][z].type) ||
                        block_top_left)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;

                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y + 1, z);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y, z);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y, z + 1);
                    triangle->uv1 = (v2f){1.0f, 0.0f};
                    triangle->uv2 = (v2f){1.0f, 1.0f};
                    triangle->uv3 = (v2f){0.0f, 1.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = side_texture;
                    triangle->color = side_color;

                    if (is_front_face_visible || block_front_left)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if ((y == 0 || !self->blocks[x][y - 1][z].type) ||
                        block_bottom_left)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;
                }

                // top face
                if (y == CHUNK_HEIGHT - 1 || !self->blocks[x][y + 1][z].type) {
                    const Texture *const top_texture =
                        block_top_textures[self->blocks[x][y][z].type];
                    const Color top_color =
                        block_top_colors[self->blocks[x][y][z].type];

                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y + 1, z);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y + 1, z + 1);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y + 1, z + 1);
                    triangle->uv1 = (v2f){0.0f, 1.0f};
                    triangle->uv2 = (v2f){0.0f, 0.0f};
                    triangle->uv3 = (v2f){1.0f, 0.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = top_texture;
                    triangle->color = top_color;

                    if (is_left_face_visible || block_top_left)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if (is_back_face_visible || block_top_back)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;

                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y + 1, z + 1);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y + 1, z);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y + 1, z);
                    triangle->uv1 = (v2f){1.0f, 0.0f};
                    triangle->uv2 = (v2f){1.0f, 1.0f};
                    triangle->uv3 = (v2f){0.0f, 1.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = top_texture;
                    triangle->color = top_color;

                    if (is_right_face_visible || block_top_right)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if (is_front_face_visible || block_top_front)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;
                }

                // bottom face
                if (y > 0 && !self->blocks[x][y - 1][z].type) {
                    const Texture *const bottom_texture =
                        block_bottom_textures[self->blocks[x][y][z].type];
                    const Color bottom_color =
                        block_bottom_colors[self->blocks[x][y][z].type];

                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y, z + 1);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y, z);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y, z);
                    triangle->uv1 = (v2f){0.0f, 1.0f};
                    triangle->uv2 = (v2f){0.0f, 0.0f};
                    triangle->uv3 = (v2f){1.0f, 0.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = bottom_texture;
                    triangle->color = bottom_color;

                    if (is_left_face_visible || block_bottom_left)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if (is_front_face_visible || block_bottom_front)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;

                    i = triangle_index_array_grow(mesh->triangles);
                    triangle = &mesh->triangles->array[i];
                    triangle->v1 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y, z);
                    triangle->v2 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x + 1, y, z + 1);
                    triangle->v3 = chunk_get_vertex_index(
                        self, mesh, vertex_indices, x, y, z + 1);
                    triangle->uv1 = (v2f){1.0f, 0.0f};
                    triangle->uv2 = (v2f){1.0f, 1.0f};
                    triangle->uv3 = (v2f){0.0f, 1.0f};
                    triangle->edges = TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V2_V3;
                    triangle->texture = bottom_texture;
                    triangle->color = bottom_color;

                    if (is_right_face_visible || block_bottom_right)
                        triangle->edges |= TRIANGLE_EDGE_V1_V2_FAR;

                    if (is_back_face_visible || block_bottom_back)
                        triangle->edges |= TRIANGLE_EDGE_V2_V3_FAR;
                }
            }
        }
    }

    log_debugf("generated mesh for chunk (%d, %d) in %f ms", self->x, self->z,
               (get_time_microseconds() - start) / 1000.0f);

    return mesh;
}

[[gnu::returns_nonnull]]
static Chunk *chunk_create(const int x, const int z, const uint32_t seed,
                           const int8_t player_index) {
    log_debugf("load chunk (%d, %d)", x, z);
    Chunk *const self = malloc_or_exit(sizeof(*self), "failed to create chunk");

    self->x = x;
    self->z = z;

#ifndef __wasm__
    pthread_mutex_init(&self->mesh_mutex, NULL);
#endif

    for (uint8_t i = 0; i < 4; ++i) {
        self->loaded_by[i] = false;
    }
    self->loaded_by[player_index] = true;

    const v2i chunk_origin = {
        .x = (WORLD_ORIGIN + self->x) * CHUNK_SIZE,
        .y = (WORLD_ORIGIN + self->z) * CHUNK_SIZE,
    };

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            const v2i block_coordinate = v2i_add(chunk_origin, (v2i){x, z});

            const float terrain_height_noise = powf(
                perlin_noise(block_coordinate, seed,
                             CHUNK_GENERATION_TERRAIN_HEIGHT_NOISE_FREQUENCY,
                             CHUNK_GENERATION_TERRAIN_HEIGHT_NOISE_DEPTH),
                CHUNK_GENERATION_TERRAIN_HEIGHT_NOISE_POW);
            assert(0.0f <= terrain_height_noise);
            assert(terrain_height_noise <= 1.0f);

            const float desert_noise =
                perlin_noise(block_coordinate, seed + 1,
                             CHUNK_GENERATION_DESERT_NOISE_FREQUENCY,
                             CHUNK_GENERATION_DESERT_NOISE_DEPTH);
            assert(0.0f <= desert_noise);
            assert(desert_noise <= 1.0f);

            const float surface_layer_thickness_noise =
                perlin_noise(block_coordinate, seed + 2,
                             CHUNK_GENERATION_SURFACE_LAYER_NOISE_FREQUENCY,
                             CHUNK_GENERATION_SURFACE_LAYER_NOISE_DEPTH);
            assert(0.0f <= surface_layer_thickness_noise &&
                   surface_layer_thickness_noise <= 1.0f);

            const float min_snow_min_height_variation_noise =
                perlin_noise(block_coordinate, seed + 3,
                             CHUNK_GENERATION_MIN_SNOW_HEIGHT_NOISE_FREQUENCY,
                             CHUNK_GENERATION_MIN_SNOW_HEIGHT_NOISE_DEPTH);
            assert(0.0f <= min_snow_min_height_variation_noise);
            assert(min_snow_min_height_variation_noise <= 1.0f);

            int max_y =
                terrain_height_noise * (CHUNK_GENERATION_MAX_TERRAIN_HEIGHT -
                                        CHUNK_GENERATION_MIN_TERRAIN_HEIGHT) +
                CHUNK_GENERATION_MIN_TERRAIN_HEIGHT;

            const int surface_layer_thickness =
                surface_layer_thickness_noise *
                    (CHUNK_GENERATION_SURFACE_LAYER_MAX_THICKNESS -
                     CHUNK_GENERATION_SURFACE_LAYER_MIN_THICKNESS) +
                CHUNK_GENERATION_SURFACE_LAYER_MIN_THICKNESS;
            assert(CHUNK_GENERATION_SURFACE_LAYER_MIN_THICKNESS <=
                   surface_layer_thickness);
            assert(surface_layer_thickness <=
                   CHUNK_GENERATION_SURFACE_LAYER_MAX_THICKNESS);

            const int max_stone_y = max_y - surface_layer_thickness;

            const bool is_desert =
                desert_noise > CHUNK_GENERATION_DESERT_NOISE_THRESHOLD;

            const int min_snow_min_height_variation =
                min_snow_min_height_variation_noise *
                    CHUNK_GENERATION_MIN_SNOW_HEIGHT_VARIATION * 2 -
                CHUNK_GENERATION_MIN_SNOW_HEIGHT_VARIATION;

            block_init(&self->blocks[x][0][z], BLOCK_TYPE_BEDROCK);
            for (int y = 1; y < CHUNK_HEIGHT; ++y) {
                BlockType block_type;
                if (y <= max_stone_y) {
                    block_type = BLOCK_TYPE_STONE;
                } else if (y <= max_y) {
                    if (y > CHUNK_GENERATION_MIN_SNOW_HEIGHT +
                                min_snow_min_height_variation) {
                        block_type = BLOCK_TYPE_SNOW;
                    } else if (is_desert) {
                        block_type = BLOCK_TYPE_SAND;
                    } else {
                        if (y == max_y) {
                            block_type = BLOCK_TYPE_GRASS;
                        } else {
                            block_type = BLOCK_TYPE_DIRT;
                        }
                    }
                } else {
                    block_type = BLOCK_TYPE_AIR;
                }
                block_init(&self->blocks[x][y][z], block_type);
            }
        }
    }

    self->mesh = NULL;

    return self;
}

[[gnu::nonnull]]
static void chunk_destroy(Chunk *const self) {
    assert(self != NULL);
#ifndef __wasm__
    mutex_destroy(&self->mesh_mutex);
#endif
    if (self->mesh != NULL) {
        mesh_destroy(self->mesh);
    }
    free(self);
}

[[gnu::nonnull]]
static void chunk_render(Chunk *const restrict self,
                         const Camera *const restrict camera,
                         const World *const restrict world,
                         const Viewport *const restrict viewport) {
    assert(self != NULL);
    assert(camera != NULL);
    assert(world != NULL);
    assert(viewport != NULL);

    mutex_lock(&self->mesh_mutex);
    if (self->mesh == NULL) {
        self->mesh = chunk_generate_mesh(self, world);
    }
    mutex_unlock(&self->mesh_mutex);
    mesh_render(self->mesh, camera, viewport);
}

[[gnu::nonnull]]
static void chunk_make_mesh_dirty(Chunk *const self) {
    assert(self != NULL);

    if (self->mesh == NULL) return;

    mesh_destroy(self->mesh);
    self->mesh = NULL;
}

World *world_create(const uint32_t seed) {
    World *const self = malloc_or_exit(sizeof(*self), "failed to create world");
    self->place_block = BLOCK_TYPE_STONE;

    log_debugf("world seed: %u", seed);
    self->seed = seed;

    for (int x = 0; x < WORLD_SIZE; ++x) {
        for (int z = 0; z < WORLD_SIZE; ++z) {
            self->chunks[x][z] = NULL;
        }
    }
    return self;
}

void world_destroy(World *const self) {
    assert(self != NULL);
    for (size_t x = 0; x < WORLD_SIZE; ++x) {
        for (size_t z = 0; z < WORLD_SIZE; ++z) {
            if (self->chunks[x][z] != NULL) {
                chunk_destroy(self->chunks[x][z]);
            }
        }
    }
    free(self);
}

v2i world_position_to_chunk_coordinate(const v3f position) {
    return (v2i){
        .x = ((int)floorf(position.x) + WORLD_ORIGIN * CHUNK_SIZE) / CHUNK_SIZE,
        .y = ((int)floorf(position.z) + WORLD_ORIGIN * CHUNK_SIZE) / CHUNK_SIZE,
    };
}

static v2i world_position_to_chunk_coordinate_v3i(const v3i position) {
    return (v2i){
        .x = (position.x + WORLD_ORIGIN * CHUNK_SIZE) / CHUNK_SIZE,
        .y = (position.z + WORLD_ORIGIN * CHUNK_SIZE) / CHUNK_SIZE,
    };
}

[[gnu::nonnull]]
static inline bool chunk_need_to_be_loaded(const Chunk *const self) {
    assert(self != NULL);
    return self->loaded_by[0] || self->loaded_by[1] || self->loaded_by[2] ||
           self->loaded_by[3];
}

void world_update_loaded_chunks(World *const self, const v2i old_chunk_position,
                                const v2i new_chunk_position,
                                const int8_t player_index) {
    assert(self != NULL);

    const int old_load_min_x =
        max_int(0, old_chunk_position.x - WORLD_LOAD_DISTANCE);
    const int old_load_max_x =
        min_int(old_chunk_position.x + WORLD_LOAD_DISTANCE + 1, WORLD_SIZE);
    const int old_load_min_z =
        max_int(0, old_chunk_position.y - WORLD_LOAD_DISTANCE);
    const int old_load_max_z =
        min_int(old_chunk_position.y + WORLD_LOAD_DISTANCE + 1, WORLD_SIZE);
    assert(0 <= old_load_max_x && old_load_max_x <= WORLD_SIZE);
    assert(0 <= old_load_max_z && old_load_max_z <= WORLD_SIZE);

    const int new_load_min_x =
        max_int(0, new_chunk_position.x - WORLD_LOAD_DISTANCE);
    const int new_load_max_x =
        min_int(new_chunk_position.x + WORLD_LOAD_DISTANCE + 1, WORLD_SIZE);
    const int new_load_min_z =
        max_int(0, new_chunk_position.y - WORLD_LOAD_DISTANCE);
    const int new_load_max_z =
        min_int(new_chunk_position.y + WORLD_LOAD_DISTANCE + 1, WORLD_SIZE);
    assert(0 <= new_load_max_x && new_load_max_x <= WORLD_SIZE);
    assert(0 <= new_load_max_z && new_load_max_z <= WORLD_SIZE);

    for (int x = old_load_min_x; x < old_load_max_x; ++x) {
        for (int z = old_load_min_z; z < old_load_max_z; ++z) {
            if (x < new_load_min_x || x >= new_load_max_x ||
                z < new_load_min_z || z >= new_load_max_z) {
                self->chunks[x][z]->loaded_by[player_index] = false;
                if (!chunk_need_to_be_loaded(self->chunks[x][z])) {
                    log_debugf("unload chunk (%d, %d)", x - WORLD_ORIGIN,
                               z - WORLD_ORIGIN);
                    chunk_destroy(self->chunks[x][z]);
                    self->chunks[x][z] = NULL;
                }
            }
        }
    }

    for (int x = new_load_min_x; x < new_load_max_x; ++x) {
        for (int z = new_load_min_z; z < new_load_max_z; ++z) {
            if (x < old_load_min_x || x >= old_load_max_x ||
                z < old_load_min_z || z >= old_load_max_z) {
                if (self->chunks[x][z] == NULL) {
                    log_debugf("load chunk (%d, %d)", x - WORLD_ORIGIN,
                               z - WORLD_ORIGIN);
                    self->chunks[x][z] =
                        chunk_create(x - WORLD_ORIGIN, z - WORLD_ORIGIN,
                                     self->seed, player_index);
                }
                self->chunks[x][z]->loaded_by[player_index] = true;
            }
        }
    }
}

#ifndef __wasm__
#ifndef WORLD_RENDER_SCHEDULER_DYNAMIC
typedef struct {
    const World *self;
    const Camera *camera;
    const Viewport *viewport;
    int min_x, min_z;
    int width;
} WorldRenderContext;

typedef struct {
    const WorldRenderContext *render_context;
    int from;
    int to;
} WorldRenderThread;

[[gnu::nonnull]]
static void *world_render_thread(void *const data) {
    assert(data != NULL);

    const WorldRenderThread *const thread = data;

    const World *const self = thread->render_context->self;
    const Camera *const camera = thread->render_context->camera;
    const Viewport *const viewport = thread->render_context->viewport;
    const int min_x = thread->render_context->min_x;
    const int min_z = thread->render_context->min_z;
    const int width = thread->render_context->width;

    for (int i = thread->from; i < thread->to; ++i) {
        const int x = min_x + i / width;
        const int z = min_z + i % width;
        Chunk *const chunk = self->chunks[x][z];
        assert(chunk != NULL);
        chunk_render(chunk, camera, self, viewport);
    }

    return NULL;
}

void world_render(const World *const restrict self,
                  const Camera *const restrict camera,
                  const Viewport *const restrict viewport) {
    assert(self != NULL);
    assert(camera != NULL);
    assert(viewport != NULL);

    const v2i camera_chunk_position =
        world_position_to_chunk_coordinate(camera->position);

    const int min_x =
        max_int(0, camera_chunk_position.x - WORLD_RENDER_DISTANCE);
    const int max_x = min_int(
        camera_chunk_position.x + WORLD_RENDER_DISTANCE + 1, WORLD_SIZE);
    const int min_z =
        max_int(0, camera_chunk_position.y - WORLD_RENDER_DISTANCE);
    const int max_z = min_int(
        camera_chunk_position.y + WORLD_RENDER_DISTANCE + 1, WORLD_SIZE);

    const int width = max_x - min_x;
    const int height = max_z - min_z;
    const int size = width * height;

    const WorldRenderContext render_context = {
        .self = self,
        .camera = camera,
        .viewport = viewport,
        .width = width,
        .min_x = min_x,
        .min_z = min_z,
    };

    const int chunkPerThread = size / WORLD_RENDER_THREADS_NUMBER;
    const int remainingChunks = size % WORLD_RENDER_THREADS_NUMBER;
    int assignedChunks = chunkPerThread + (0 < remainingChunks);

    WorldRenderThread threads_data[WORLD_RENDER_THREADS_NUMBER];

    threads_data[0].render_context = &render_context;
    threads_data[0].from = 0;
    threads_data[0].to = assignedChunks;

    pthread_t threads[WORLD_RENDER_THREADS_NUMBER - 1];

    for (size_t i = 0; i < WORLD_RENDER_THREADS_NUMBER - 1; ++i) {
        const int j = i + 1;
        threads_data[j].render_context = &render_context;
        threads_data[j].from = assignedChunks;
        assignedChunks += chunkPerThread + (j < remainingChunks);
        threads_data[j].to = assignedChunks;
        const int return_code = pthread_create(
            &threads[i], NULL, world_render_thread, &threads_data[j]);
        if (return_code != 0) {
            log_errorf("failed to create render thread: %s",
                       strerror(return_code));
            exit(EXIT_FAILURE);
        }
    }

    world_render_thread(&threads_data[0]);

    for (size_t i = 0; i < WORLD_RENDER_THREADS_NUMBER - 1; ++i) {
        const int return_code = pthread_join(threads[i], NULL);
        if (return_code != 0) {
            log_errorf("failed to join render thread: %s",
                       strerror(return_code));
            exit(EXIT_FAILURE);
        }
    }
}

#else

typedef struct {
    const World *self;
    const Camera *camera;
    const Viewport *viewport;
    pthread_mutex_t mutex;
    int x, z;
    int min_x, max_x;
    int min_z, max_z;
} WorldRenderContext;

static void *world_render_thread(void *const data) NONNULL();

static void *world_render_thread(void *const data) {
    assert(data != NULL);

    WorldRenderContext *const render_context = data;

    const World *const self = render_context->self;
    const Camera *const camera = render_context->camera;
    const Viewport *const viewport = render_context->viewport;

    while (true) {
        mutex_lock(&render_context->mutex);
        const int x = render_context->x;
        if (x >= render_context->max_x) {
            mutex_unlock(&render_context->mutex);
            break;
        }
        const int z = render_context->z++;
        if (render_context->z >= render_context->max_z) {
            ++render_context->x;
            render_context->z = render_context->min_z;
        }
        mutex_unlock(&render_context->mutex);

        Chunk *const chunk = self->chunks[x][z];
        assert(chunk != NULL);
        chunk_render(chunk, camera, self, viewport);
    }

    return NULL;
}

void world_render(const world *const restrict self,
                  const camera *const restrict camera,
                  const viewport *const restrict viewport) {
    assert(self != NULL);
    assert(camera != NULL);
    assert(viewport != NULL);

    const v2i camera_chunk_position =
        world_position_to_chunk_coordinate(camera->position);

    const int min_x =
        max_int(0, camera_chunk_position.x - WORLD_RENDER_DISTANCE);
    const int max_x = min_int(
        camera_chunk_position.x + WORLD_RENDER_DISTANCE + 1, WORLD_SIZE);
    const int min_z =
        max_int(0, camera_chunk_position.y - WORLD_RENDER_DISTANCE);
    const int max_z = min_int(
        camera_chunk_position.y + WORLD_RENDER_DISTANCE + 1, WORLD_SIZE);

    WorldRenderContext render_context = {
        .self = self,
        .camera = camera,
        .viewport = viewport,
        .x = min_x,
        .z = min_z,
        .min_x = min_x,
        .max_x = max_x,
        .min_z = min_z,
        .max_z = max_z,
    };
    pthread_mutex_init(&render_context.mutex, NULL);

    pthread_t threads[WORLD_RENDER_THREADS_NUMBER - 1];

    for (size_t i = 0; i < WORLD_RENDER_THREADS_NUMBER - 1; ++i) {
        const int return_code = pthread_create(
            &threads[i], NULL, world_render_thread, &render_context);
        if (return_code != 0) {
            log_errorf("failed to create render thread: %s",
                       strerror(return_code));
            exit(EXIT_FAILURE);
        }
    }

    world_render_thread(&render_context);

    for (size_t i = 0; i < WORLD_RENDER_THREADS_NUMBER - 1; ++i) {
        const int return_code = pthread_join(threads[i], NULL);
        if (return_code != 0) {
            log_errorf("failed to join render thread: %s",
                       strerror(return_code));
            exit(EXIT_FAILURE);
        }
    }

    mutex_destroy(&render_context.mutex);
}
#endif
#else
void world_render(const World *const restrict self,
                  const Camera *const restrict camera,
                  const Viewport *const restrict viewport) {
    assert(self != NULL);
    assert(camera != NULL);
    assert(viewport != NULL);

    const v2i camera_chunk_position =
        world_position_to_chunk_coordinate(camera->position);

    const int min_x =
        max_int(0, camera_chunk_position.x - WORLD_RENDER_DISTANCE);
    const int max_x = min_int(
        camera_chunk_position.x + WORLD_RENDER_DISTANCE + 1, WORLD_SIZE);
    const int min_z =
        max_int(0, camera_chunk_position.y - WORLD_RENDER_DISTANCE);
    const int max_z = min_int(
        camera_chunk_position.y + WORLD_RENDER_DISTANCE + 1, WORLD_SIZE);

    for (int z = min_z; z < max_z; ++z) {
        for (int x = min_x; x < max_x; ++x) {
            Chunk *const chunk = self->chunks[x][z];
            assert(chunk != NULL);
            chunk_render(chunk, camera, self, viewport);
        }
    }
}
#endif

Chunk *world_get_chunk(const World *const self, const int x, const int z) {
    assert(self != NULL);

    const int x_index = x + WORLD_ORIGIN;
    const int z_index = z + WORLD_ORIGIN;

    if (x_index < 0 || x_index >= WORLD_SIZE || z_index < 0 ||
        z_index >= WORLD_SIZE) {
        return NULL;
    }

    return self->chunks[x_index][z_index];
}

Block *world_get_block(const World *const self, const v3i block_position) {
    assert(self != NULL);
    assert(0 <= block_position.y && block_position.y < CHUNK_HEIGHT);

    const int shifted_x = block_position.x + WORLD_ORIGIN * CHUNK_SIZE;
    const int shifted_z = block_position.z + WORLD_ORIGIN * CHUNK_SIZE;

    assert(0 <= shifted_x && 0 <= shifted_z);

    const int chunk_x = shifted_x / CHUNK_SIZE;
    const int chunk_z = shifted_z / CHUNK_SIZE;

    assert(chunk_x < WORLD_SIZE && chunk_z < WORLD_SIZE);

    const int x_index = POSITIVE_MOD(block_position.x, CHUNK_SIZE);
    const int z_index = POSITIVE_MOD(block_position.z, CHUNK_SIZE);

    assert(self->chunks[chunk_x][chunk_z] != NULL);

    return &self->chunks[chunk_x][chunk_z]
                ->blocks[x_index][block_position.y][z_index];
}

bool world_block_is_solid(const World *const self, const v3i block_position) {
    assert(self != NULL);

    if (block_position.y < 0 || block_position.y >= CHUNK_HEIGHT) return false;

    const int shifted_x = block_position.x + WORLD_ORIGIN * CHUNK_SIZE;
    const int shifted_z = block_position.z + WORLD_ORIGIN * CHUNK_SIZE;

    if (shifted_x < 0 || shifted_z < 0) return true;

    const int chunk_x = shifted_x / CHUNK_SIZE;
    const int chunk_z = shifted_z / CHUNK_SIZE;

    if (chunk_x >= WORLD_SIZE || chunk_z >= WORLD_SIZE) return true;

    const int x_index = POSITIVE_MOD(block_position.x, CHUNK_SIZE);
    const int z_index = POSITIVE_MOD(block_position.z, CHUNK_SIZE);

    assert(self->chunks[chunk_x][chunk_z] != NULL);
    return self->chunks[chunk_x][chunk_z]
        ->blocks[x_index][block_position.y][z_index]
        .type;
}

void world_load_chunks_around_player(World *const self,
                                     const v2i player_chunk_position,
                                     const int8_t player_index) {
    assert(self != NULL);

    const int min_x = max_int(0, player_chunk_position.x - WORLD_LOAD_DISTANCE);
    const int max_x =
        min_int(player_chunk_position.x + WORLD_LOAD_DISTANCE + 1, WORLD_SIZE);
    const int min_z = max_int(0, player_chunk_position.y - WORLD_LOAD_DISTANCE);
    const int max_z =
        min_int(player_chunk_position.y + WORLD_LOAD_DISTANCE + 1, WORLD_SIZE);

    for (int x = min_x; x < max_x; ++x) {
        for (int z = min_z; z < max_z; ++z) {
            if (self->chunks[x][z] == NULL) {
                self->chunks[x][z] =
                    chunk_create(x - WORLD_ORIGIN, z - WORLD_ORIGIN, self->seed,
                                 player_index);
            } else if (!self->chunks[x][z]->loaded_by[player_index]) {
                self->chunks[x][z]->loaded_by[player_index] = true;
            }
        }
    }
}

[[gnu::nonnull(1, 2)]]
static void world_update_chunk_mesh_around_block(World *const restrict self,
                                                 Chunk *const restrict chunk,
                                                 const uint8_t x_index,
                                                 const uint8_t z_index) {
    assert(self != NULL);
    assert(chunk != NULL);

    chunk_make_mesh_dirty(chunk);

    const int chunk_x = chunk->x + WORLD_ORIGIN;
    const int chunk_z = chunk->z + WORLD_ORIGIN;

    if (x_index == 0) {
        if (chunk_x != 0) {
            chunk_make_mesh_dirty(self->chunks[chunk_x - 1][chunk_z]);
            if (z_index == 0) {
                if (chunk_z != 0) {
                    chunk_make_mesh_dirty(
                        self->chunks[chunk_x - 1][chunk_z - 1]);
                }
            } else if (z_index == CHUNK_SIZE - 1 && chunk_z != WORLD_SIZE - 1) {
                chunk_make_mesh_dirty(self->chunks[chunk_x - 1][chunk_z + 1]);
            }
        }
    } else if (x_index == CHUNK_SIZE - 1) {
        if (chunk_x != WORLD_SIZE - 1) {
            chunk_make_mesh_dirty(self->chunks[chunk_x + 1][chunk_z]);
            if (z_index == 0) {
                if (chunk_z != 0) {
                    chunk_make_mesh_dirty(
                        self->chunks[chunk_x + 1][chunk_z - 1]);
                }
            } else if (z_index == CHUNK_SIZE - 1 && chunk_z != WORLD_SIZE - 1) {
                chunk_make_mesh_dirty(self->chunks[chunk_x + 1][chunk_z + 1]);
            }
        }
    }

    if (z_index == 0) {
        if (chunk_z != 0) {
            chunk_make_mesh_dirty(self->chunks[chunk_x][chunk_z - 1]);
        }
    } else if (z_index == CHUNK_SIZE - 1 && chunk_z != WORLD_SIZE - 1) {
        chunk_make_mesh_dirty(self->chunks[chunk_x][chunk_z + 1]);
    }
}

void world_place_block(World *const self, const v3i block_position) {
    assert(self != NULL);

    const v2i chunk_position =
        world_position_to_chunk_coordinate_v3i(block_position);
    assert(0 <= chunk_position.x && chunk_position.x < WORLD_SIZE);
    assert(0 <= chunk_position.y && chunk_position.y < WORLD_SIZE);

    Chunk *const chunk = self->chunks[chunk_position.x][chunk_position.y];
    assert(chunk != NULL);

    const int x_index = POSITIVE_MOD(block_position.x, CHUNK_SIZE);
    const int z_index = POSITIVE_MOD(block_position.z, CHUNK_SIZE);

    assert(chunk->blocks[x_index][block_position.y][z_index].type ==
           BLOCK_TYPE_AIR);

    chunk->blocks[x_index][block_position.y][z_index].type = self->place_block;

    world_update_chunk_mesh_around_block(self, chunk, x_index, z_index);
}

void world_break_block(World *const self, const v3i block_position) {
    const int chunk_x =
        (block_position.x + WORLD_ORIGIN * CHUNK_SIZE) / CHUNK_SIZE;
    const int chunk_z =
        (block_position.z + WORLD_ORIGIN * CHUNK_SIZE) / CHUNK_SIZE;
    assert(0 <= chunk_x && chunk_x < WORLD_SIZE);
    assert(0 <= chunk_z && chunk_z < WORLD_SIZE);

    Chunk *const chunk = self->chunks[chunk_x][chunk_z];
    assert(chunk != NULL);

    const int x_index = POSITIVE_MOD(block_position.x, CHUNK_SIZE);
    const int z_index = POSITIVE_MOD(block_position.z, CHUNK_SIZE);

    assert(chunk->blocks[x_index][block_position.y][z_index].type !=
           BLOCK_TYPE_AIR);

    if (chunk->blocks[x_index][block_position.y][z_index].type ==
        BLOCK_TYPE_BEDROCK)
        return;

    chunk->blocks[x_index][block_position.y][z_index].type = BLOCK_TYPE_AIR;

    world_update_chunk_mesh_around_block(self, chunk, x_index, z_index);
}
