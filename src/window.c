#include "window.h"

#include <assert.h>
#include <float.h>
#ifndef __wasm__
#include <signal.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "config.h"
#include "event_queue.h"
#include "log.h"
#include "text.h"
#include "texture.h"
#include "threads.h"
#include "utils.h"
#include "wasm/js.h"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define RESET_TEXT_STYLE "\033[0m"
#define TEXT_BOLD "\033[1m"
#define MOVE_CURSOR_TOP_LEFT "\033[H"
#define SWITCH_TO_ALTERNATE_SCREEN "\033[?1049h"
#define SWITCH_TO_REGULAR_SCREEN "\033[?1049l"

#define PIXEL_MAX_SIZE 6      // color: 5, char: 1
#define PIXEL_MAX_SIZE_TTY 9  // color: 8, char: 1

#define CURSOR_POSITION_BUFFER_CAPACITY 27

#define DISPLAY_BUFFER_SIZE(display_buffer, window_size, pixel_max_size) \
    (sizeof(*display_buffer) * (pixel_max_size) * (window_size) +        \
     sizeof(MOVE_CURSOR_TOP_LEFT) - 1 + sizeof(HIDE_CURSOR) - 1 +        \
     sizeof(SHOW_CURSOR) - 1 + sizeof(TEXT_BOLD) - 1 +                   \
     CURSOR_POSITION_BUFFER_CAPACITY)

#define WINDOW_FD STDOUT_FILENO

#define WRITE(string_literal) \
    write(WINDOW_FD, (string_literal), sizeof((string_literal)) - 1)

Window window = {
    .width = 0,
    .height = 0,
    .pixels = NULL,
    .display_buffer = NULL,
    .cursor_position = {0, 0},
    .show_cursor = false,
#ifndef NDEBUG
    .is_init = false,
#endif
};

#ifndef __wasm__
static void window_restore_terminal_attr(void) {
    assert(window.is_init);
    if (tcsetattr(WINDOW_FD, TCSANOW, &window.old_attr)) {
        log_errorf_errno(TCSETATTR_FAILED_MESSAGE);
        exit(EXIT_FAILURE);
    }
}
#else
#define window_restore_terminal_attr()
#endif

[[gnu::nonnull]]
static void get_terminal_size(int *const restrict width,
                              int *const restrict height) {
    assert(width != NULL);
    assert(height != NULL);

    struct winsize window_size;
    if (ioctl(WINDOW_FD, TIOCGWINSZ, &window_size)) {
        log_errorf_errno("failed to get the terminal size");
        exit(EXIT_FAILURE);
    }
    *width = window_size.ws_col;
    *height = window_size.ws_row;
}

static void window_show_cursor(void) {
    if (WRITE(SHOW_CURSOR) < 0) {
        log_errorf_errno("failed to show cursor: write failed");
        exit(EXIT_FAILURE);
    }
}

static void window_hide_cursor(void) {
    if (WRITE(HIDE_CURSOR) < 0) {
        log_errorf_errno("failed to hide cursor: write failed");
        exit(EXIT_FAILURE);
    }
}

static void window_reset_text_style(void) {
    if (WRITE(RESET_TEXT_STYLE) < 0) {
        log_errorf_errno("failed to reset text style: write failed");
        exit(EXIT_FAILURE);
    }
}

#ifndef __wasm__
static void handle_sigcont([[gnu::unused]] const int _sig) {
    log_debugf("SIGCONT received");
    window_show_cursor();
}
#endif

static bool is_run_in_tty(void) {
    const char *const tty_name = ttyname(WINDOW_FD);
    if (tty_name == NULL) {
        log_errorf_errno("failed to get tty name");
        exit(EXIT_FAILURE);
    }
    const bool is_tty =
        strncmp(tty_name, "/dev/tty", sizeof("/dev/tty") - 1) == 0;
    if (!is_tty) {
        const char *const term = getenv("TERM");
        return term != NULL && strcmp(term, "linux") == 0;
    }
    return true;
}

void window_init(const bool force_tty, const bool force_no_tty) {
    assert(!window.is_init);

    if (force_tty) {
        window.is_run_in_tty = true;
        log_debugf("force tty mode");
    } else if (force_no_tty) {
        window.is_run_in_tty = false;
        log_debugf("force not tty mode");
    } else {
        window.is_run_in_tty = is_run_in_tty();
        log_debugf("running in a tty: %s", BOOL_TO_STR(window.is_run_in_tty));
    }

    get_terminal_size(&window.width, &window.height);
    log_debugf("window size: (%d, %d)", window.width, window.height);

    const size_t window_size = window.width * window.height;
    window.pixels = malloc_or_exit(sizeof(*window.pixels) * window_size,
                                   "failed to create window pixels buffer");
    window.display_buffer = malloc_or_exit(
        DISPLAY_BUFFER_SIZE(
            window.display_buffer, window_size,
            window.is_run_in_tty ? PIXEL_MAX_SIZE_TTY : PIXEL_MAX_SIZE),
        "failed to create window display buffer");

    if (WRITE(SWITCH_TO_ALTERNATE_SCREEN) < 0) {
        log_errorf_errno("failed to switch to alternate screen: write failed");
        exit(EXIT_FAILURE);
    }

#ifndef __wasm__
    if (tcgetattr(WINDOW_FD, &window.old_attr)) {
        log_errorf_errno(TCGETATTR_FAILED_MESSAGE);
        exit(EXIT_FAILURE);
    }

    struct termios attr;
    if (tcgetattr(WINDOW_FD, &attr)) {
        log_errorf_errno(TCGETATTR_FAILED_MESSAGE);
        exit(EXIT_FAILURE);
    }
    attr.c_iflag &= ~(ICRNL | IXON);
    attr.c_oflag &= ~(OPOST);
    attr.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
    attr.c_cc[VMIN] = 0;
    if (tcsetattr(WINDOW_FD, TCSANOW, &attr)) {
        log_errorf_errno(TCSETATTR_FAILED_MESSAGE);
        exit(EXIT_FAILURE);
    }
#endif

    window_hide_cursor();

#ifndef __wasm__
    if (signal(SIGCONT, handle_sigcont) == SIG_ERR) {
        log_errorf_errno("failed to setup SIGCONT handler");
        exit(EXIT_FAILURE);
    }
#endif

    window.cursor_position.x = 0;
    window.cursor_position.y = 0;
    window.show_cursor = false;

#ifndef NDEBUG
    window.is_init = true;
#endif
}

void window_quit(void) {
    assert(window.is_init);
    window_show_cursor();
    window_reset_text_style();
    window_restore_terminal_attr();

    if (WRITE(SWITCH_TO_REGULAR_SCREEN) < 0) {
        log_errorf_errno("failed to switch to regular screen: write failed");
        exit(EXIT_FAILURE);
    }

#ifndef __wasm__
    const size_t window_size = window.width * window.height;
    for (size_t i = 0; i < window_size; ++i) {
        mutex_destroy(&window.pixels[i].mutex);
    }
#endif
    free(window.pixels);
    free(window.display_buffer);
#ifndef NDEBUG
    window.is_init = false;
#endif
}

static inline void window_detect_resize(void) {
    assert(window.is_init);
    int width, height;
    get_terminal_size(&width, &height);
    if (window.width != width || window.height != height) {
        log_debugf("resize window from (%d, %d) to (%d, %d)", window.width,
                   window.height, width, height);
#ifndef __wasm__
        const size_t old_window_size = window.width * window.height;
        for (size_t i = 0; i < old_window_size; ++i) {
            mutex_destroy(&window.pixels[i].mutex);
        }
#endif
        const size_t new_window_size = width * height;
        window.pixels = realloc_or_exit(
            window.pixels, sizeof(*window.pixels) * new_window_size,
            "failed to resize window pixels buffer");
#ifndef __wasm__
        for (size_t i = 0; i < new_window_size; ++i) {
            pthread_mutex_init(&window.pixels[i].mutex, NULL);
        }
#endif
        window.display_buffer = realloc_or_exit(
            window.display_buffer,
            DISPLAY_BUFFER_SIZE(
                window.display_buffer, new_window_size,
                window.is_run_in_tty ? PIXEL_MAX_SIZE_TTY : PIXEL_MAX_SIZE),
            "failed to resize window display buffer");

        window.width = width;
        window.height = height;

        event_queue_push(&(Event){.type = EVENT_TYPE_RESIZE});
    }
}

static inline void handle_keyboard_input(void) {
    while (true) {
        char chr;
        const int64_t result = read(STDIN_FILENO, &chr, 1);
        if (result < 0) {
            log_errorf_errno("failed to read from stdin");
            exit(EXIT_FAILURE);
        }

        if (!result) break;

        log_debugf("char event: 0x%02x", chr);
        event_queue_push(&(Event){
            .type = EVENT_TYPE_CHAR,
            .char_event = {.chr = chr},
        });
    }
}

void window_update(void) {
    assert(window.is_init);
#ifdef __wasm__
    JS_wait_for_next_frame();
#endif
    window_detect_resize();
    handle_keyboard_input();
}

void window_clear(void) {
    assert(window.is_init);
    const size_t window_size = window.width * window.height;
    for (size_t i = 0; i < window_size; ++i) {
        window_set_pixel(i, WINDOW_CLEAR_CHAR, WINDOW_CLEAR_COLOR, FLT_MAX);
    }
}

#define ANSI_ESCAPE(color) "\033[" #color "m"

void window_flush(void) {
    assert(window.is_init);

    static const char *const colors_normal[COLOR_COUNT] = {
#define COLOR(name, code) [COLOR_##name] = "\033[" #code "m",
        COLORS
#undef COLOR
    };

    static const char *const colors_tty[COLOR_COUNT] = {
#define COLOR(name, code) [COLOR_##name] = "\033[22;" #code "m",
        COLORS
#undef COLOR
    };

#define DISPLAY_BUFFER_APPEND(string)                                 \
    do {                                                              \
        for (uint8_t j = 0; string[j]; ++j)                           \
            window.display_buffer[display_buffer_size++] = string[j]; \
    } while (0)

    size_t display_buffer_size = 0;
    const char *const *colors = colors_normal;
    if (window.is_run_in_tty) {
        colors = colors_tty;
    } else {
        DISPLAY_BUFFER_APPEND(TEXT_BOLD);
    }
    DISPLAY_BUFFER_APPEND(HIDE_CURSOR);
    DISPLAY_BUFFER_APPEND(MOVE_CURSOR_TOP_LEFT);
    DISPLAY_BUFFER_APPEND(colors[window.pixels[0].color]);
    Color last_color = window.pixels[0].color;
    const size_t window_size = window.width * window.height;
    for (size_t i = 0; i < window_size; ++i) {
        const Pixel *const pixel = &window.pixels[i];
        const Color color = pixel->color;
        assert(color < COLOR_COUNT);
        if (color != last_color) {
            DISPLAY_BUFFER_APPEND(colors[color]);
            last_color = color;
        }
        window.display_buffer[display_buffer_size++] = pixel->chr;
    }
    if (window.show_cursor) {
        char cursor_position_buffer[CURSOR_POSITION_BUFFER_CAPACITY];
        snprintf(cursor_position_buffer, CURSOR_POSITION_BUFFER_CAPACITY,
                 "\033[%d;%dH", window.cursor_position.y + 1,
                 window.cursor_position.x + 1);
        DISPLAY_BUFFER_APPEND(cursor_position_buffer);
        DISPLAY_BUFFER_APPEND(SHOW_CURSOR);
    }

    if (write(WINDOW_FD, window.display_buffer, display_buffer_size) < 0) {
        log_errorf_errno("failed to flush window: write failed");
        exit(EXIT_FAILURE);
    }
}

void window_render_rectangle(const v2i position, const v2i size, const char chr,
                             const Color color, const float z) {
    assert(window.is_init);

    const int min_x = max_int(0, position.x);
    const int max_x = min_int(window.width, position.x + size.x);
    const int max_y = min_int(position.y + size.y, window.height);
    for (int y = max_int(0, position.y); y < max_y; ++y) {
        const int row_offset = y * window.width;
        for (int x = min_x; x < max_x; ++x) {
            const int index = row_offset + x;
            window_set_pixel(index, chr, color, z);
        }
    }
}

static char line_get_char(const float dx, const float dy) {
    if (dx == 0.0f) return '|';

    const float dir = fabsf((float)dy / dx);

    // TODO maybe put this in the config because it depends on the terminal font
    if (dir < 0.2f) return '-';
    if (dir > 2.0f) return '|';
    if (dx * dy < 0.0f) return '/';

    return '\\';
}

static inline void window_set_pixel_with_z_check(const int pixel_index,
                                                 const char chr,
                                                 const Color color,
                                                 const float z) {
    assert(window.is_init);
    assert(0 <= pixel_index && pixel_index < window.width * window.height);
    mutex_lock(&window.pixels[pixel_index].mutex);
    if (z < window.pixels[pixel_index].z)
        window_set_pixel(pixel_index, chr, color, z);
    mutex_unlock(&window.pixels[pixel_index].mutex);
}

static void window_render_line(v3f v1, v3f v2, const Color color) {
    assert(window.is_init);

    const float dxf = v2.x - v1.x;
    const float dyf = v2.y - v1.y;

    if (dxf == 0.0f && dyf == 0.0f) {
        const int x = (int)v1.x;
        const int y = (int)v1.y;
        if (0 <= x && x < window.width && 0 <= y && y < window.height) {
            const float z = fminf(v1.z, v2.z);
            window_set_pixel_with_z_check(
                y * window.width + x, '-', color,
                z - MESH_OUTLINE_Z_CORRECTION * (1.0f - z));
        }
        return;
    }

    const char line_char = line_get_char(dxf, dyf);

    if (fabsf(dxf) > fabsf(dyf)) {
        if (dxf < 0.0f) {
            const v3f tmp = v1;
            v1 = v2;
            v2 = tmp;
        }

        const int x_start = roundf(v1.x);
        const int x_end = v2.x;
        const float inv_dx = 1.0f / (x_end - x_start);

        const float z1 = v1.z;
        const float z2 = v2.z;

        const float y_step = dyf / dxf;

        float yf = v1.y;
        for (int x = x_start; x <= x_end; ++x) {
            const int y = (int)roundf(yf);
            if (0 <= x && x < window.width && 0 <= y && y < window.height) {
                const float t = (x - x_start) * inv_dx;
                const float z = lerp(z1, z2, t);
                window_set_pixel_with_z_check(
                    y * window.width + x, line_char, color,
                    z - MESH_OUTLINE_Z_CORRECTION * (1.0f - z));
            }
            yf += y_step;
        }
    } else {
        if (dyf < 0.0f) {
            const v3f tmp = v1;
            v1 = v2;
            v2 = tmp;
        }

        const int y_start = roundf(v1.y);
        const int y_end = v2.y;
        const float inv_dy = 1.0f / (y_end - y_start);

        const float z1 = v1.z;
        const float z2 = v2.z;

        const float x_step = dxf / dyf;

        float xf = v1.x;
        for (int y = y_start; y <= y_end; ++y) {
            const int x = (int)roundf(xf);
            if (0 <= x && x < window.width && 0 <= y && y < window.height) {
                const float t = (y - y_start) * inv_dy;
                const float z = lerp(z1, z2, t);
                window_set_pixel_with_z_check(
                    y * window.width + x, line_char, color,
                    z - MESH_OUTLINE_Z_CORRECTION * (1.0f - z));
            }
            xf += x_step;
        }
    }
}

#ifndef RENDER_WIREFRAME
static bool edge_is_top_left(const v2f v1, const v2f v2) {
    const v2f edge = v2f_sub(v2, v1);
    return edge.y < 0.0f || (edge.y == 0.0f && edge.x > 0.0f);
}

[[gnu::nonnull]]
static void window_render_triangle_fill(const Triangle3D *const triangle) {
    assert(window.is_init);
    assert(triangle != NULL);

    const float x1 = triangle->v1.x;
    const float x2 = triangle->v2.x;
    const float x3 = triangle->v3.x;
    const float y1 = triangle->v1.y;
    const float y2 = triangle->v2.y;
    const float y3 = triangle->v3.y;

    const v2f v1 = {x1, y1};
    const v2f v2 = {x2, y2};
    const v2f v3 = {x3, y3};

    const float area = v2f_get_determinant(v1, v2, v3);
    if (area == 0.0f) return;
    const float inv_area = 1.0f / area;

    const int xmin = min3_float(x1, x2, x3);
    const int ymin = min3_float(y1, y2, y3);
    const int xmax = min_int(window.width - 1, (int)max3_float(x1, x2, x3));
    const int ymax = min_int(window.height - 1, (int)max3_float(y1, y2, y3));

    assert(xmin >= 0);
    assert(ymin >= 0);

    const float z1 = triangle->v1.z;
    const float z2 = triangle->v2.z;
    const float z3 = triangle->v3.z;

    const float triangle_w1 = triangle->v1.w;
    const float triangle_w2 = triangle->v2.w;
    const float triangle_w3 = triangle->v3.w;

    const char shade = triangle->shade;
    const Color color = triangle->color;
    const Texture *const texture = triangle->texture;

    const float delta_w1_x = x3 - x2;
    const float delta_w2_x = x1 - x3;
    const float delta_w3_x = x2 - x1;
    const float delta_w1_y = y2 - y3;
    const float delta_w2_y = y3 - y1;
    const float delta_w3_y = y1 - y2;

    const float bias1 = edge_is_top_left(v2, v3) * -0.000001f;
    const float bias2 = edge_is_top_left(v3, v1) * -0.000001f;
    const float bias3 = edge_is_top_left(v1, v2) * -0.000001f;

    const v2f p0 = {xmin, ymin};
    float w1_row = v2f_get_determinant(v2, v3, p0) + bias1;
    float w2_row = v2f_get_determinant(v3, v1, p0) + bias2;
    float w3_row = v2f_get_determinant(v1, v2, p0) + bias3;

    for (int y = ymin; y <= ymax; ++y) {
        float w1 = w1_row;
        float w2 = w2_row;
        float w3 = w3_row;
        const int row_offset = y * window.width;
        for (int x = xmin; x <= xmax; ++x) {
            if (w1 >= 0 && w2 >= 0 && w3 >= 0) {
                const float alpha = w1 * inv_area;
                const float beta = w2 * inv_area;
                const float gamma = w3 * inv_area;

                Color pixel_color;
                if (texture != NULL) {
                    const float u =
                        (triangle->uv1.x * alpha * z1 / triangle_w1) +
                        (triangle->uv2.x * beta * z2 / triangle_w2) +
                        (triangle->uv3.x * gamma * z3 / triangle_w3);
                    const float v =
                        (triangle->uv1.y * alpha * z1 / triangle_w1) +
                        (triangle->uv2.y * beta * z2 / triangle_w2) +
                        (triangle->uv3.y * gamma * z3 / triangle_w3);
                    const float inv_w = (alpha / triangle_w1) +
                                        (beta / triangle_w2) +
                                        (gamma / triangle_w3);
                    pixel_color = texture_get(texture, u / inv_w, v / inv_w);
                } else {
                    pixel_color = color;
                }
                window_set_pixel_with_z_check(
                    row_offset + x, shade, pixel_color,
                    ((z1 * alpha) + (z2 * beta) + (z3 * gamma)) /
                        (alpha + beta + gamma));
            }
            w1 += delta_w1_y;
            w2 += delta_w2_y;
            w3 += delta_w3_y;
        }
        w1_row += delta_w1_x;
        w2_row += delta_w2_x;
        w3_row += delta_w3_x;
    }

    if (triangle->edges & (TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V1_V2_FAR))
        window_render_line(triangle->v1.xyz, triangle->v2.xyz,
                           MESH_OUTLINE_COLOR);

    if (triangle->edges & (TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V2_V3_FAR))
        window_render_line(triangle->v2.xyz, triangle->v3.xyz,
                           MESH_OUTLINE_COLOR);

    if (triangle->edges & (TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V3_V1_FAR))
        window_render_line(triangle->v3.xyz, triangle->v1.xyz,
                           MESH_OUTLINE_COLOR);
}
#endif

#ifdef RENDER_WIREFRAME
static void window_render_triangle_wireframe(const Triangle3D *const triangle)
    NONNULL();

static void window_render_triangle_wireframe(const Triangle3D *const triangle) {
    assert(window.is_init);
    assert(triangle != NULL);
    window_render_line(triangle->v1.xyz, triangle->v2.xyz, triangle->color);
    window_render_line(triangle->v2.xyz, triangle->v3.xyz, triangle->color);
    window_render_line(triangle->v3.xyz, triangle->v1.xyz, triangle->color);
}
#endif

void window_render_triangle(const Triangle3D *const triangle) {
    assert(window.is_init);
    assert(triangle != NULL);

#ifndef RENDER_WIREFRAME
    window_render_triangle_fill(triangle);
#else
    window_render_triangle_wireframe(triangle);
#endif
}

void window_render_string(const v2i position, const char *const string,
                          const Color color, const float z) {
    assert(window.is_init);
    assert(string != NULL);
    assert(0 <= position.x);
    assert(0 <= position.y && position.y < window.height);

    const int position_index = position.y * window.width + position.x;
    for (int i = 0; string[i]; ++i) {
        if (position.x + i < 0) continue;
        if (position.x + i >= window.width) break;
        window_set_pixel(position_index + i, string[i], color, z);
    }
}
