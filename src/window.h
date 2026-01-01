#pragma once

#include <assert.h>
#include <stdbool.h>
#ifndef __wasm__
#include <pthread.h>
#include <termios.h>
#endif

#include "color.h"
#include "triangle.h"

#define WINDOW_Z_BUFFER_FRONT -1.0f

typedef struct {
#ifndef __wasm__
    pthread_mutex_t mutex;
#endif
    float z;
    char chr;
    Color color;
} Pixel;

typedef struct {
    int width, height;
    Pixel *pixels;
    char *display_buffer;
    v2i cursor_position;
#ifndef __wasm__
    struct termios old_attr;
#endif
    bool show_cursor;
    bool is_run_in_tty;
#ifndef NDEBUG
    bool is_init;
#endif
} Window;

extern Window window;

void window_init(const bool force_tty, const bool force_no_tty);
void window_quit(void);
void window_update(void);
void window_clear(void);
void window_flush(void);

void window_render_rectangle(const v2i position, const v2i size, const char chr,
                             const Color color, const float z);

[[gnu::nonnull]]
void window_render_triangle(const Triangle3D *const triangle);

[[gnu::nonnull(2)]]
void window_render_string(const v2i position, const char *const string,
                          const Color color, const float z);

static inline void window_set_pixel(const int pixel_index, const char chr,
                                    const Color color, const float z) {
    assert(window.is_init);
    assert(0 <= pixel_index && pixel_index < window.width * window.height);
    window.pixels[pixel_index].chr = chr;
    window.pixels[pixel_index].color = color;
    window.pixels[pixel_index].z = z;
}
