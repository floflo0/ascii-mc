#pragma once

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <termios.h>

#include "color.h"
#include "triangle.h"

#define WINDOW_Z_BUFFER_FRONT -1.0f

typedef struct {
    pthread_mutex_t mutex;
    float z;
    char chr;
    Color color;
} Pixel;

typedef struct {
    int width, height;
    Pixel *pixels;
    char *display_buffer;
    v2i cursor_position;
    struct termios old_attr;
    bool show_cursor;
#ifndef NDEBUG
    bool is_init;
#endif
} Window;

extern Window window;

void window_init(void);
void window_quit(void);
void window_update(void);
void window_clear(void);
void window_flush(void);

void window_render_rectangle(const v2i position, const v2i size, const char chr,
                             const Color color, const float z);

void window_render_triangle(const Triangle3D *const triangle) NONNULL();

void window_render_string(const v2i position, const char *const string,
                          const Color color, const float z) NONNULL(2);

static inline void window_set_pixel(const int pixel_index, const char chr,
                                    const Color color, const float z) {
    assert(window.is_init);
    assert(0 <= pixel_index && pixel_index < window.width * window.height);
    window.pixels[pixel_index].chr = chr;
    window.pixels[pixel_index].color = color;
    window.pixels[pixel_index].z = z;
}
