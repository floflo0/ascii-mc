#include "window.h"

#include <assert.h>
#include <float.h>
#include <signal.h>
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
#include "threads.h"
#include "utils.h"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define MOVE_CURSOR_TOP_LEFT "\033[H"
#define SWITCH_TO_ALTERNATE_SCREEN "\033[?1049h"
#define SWITCH_TO_REGULAR_SCREEN "\033[?1049l"

#define PIXEL_MAX_SIZE 6  // color: 5, char: 1

#define CURSOR_POSITION_BUFFER_CAPACITY 27

#define DISPLAY_BUFFER_SIZE(display_buffer, window_size)          \
    (sizeof(*display_buffer) * PIXEL_MAX_SIZE * window_size +     \
     sizeof(MOVE_CURSOR_TOP_LEFT) - 1 + sizeof(HIDE_CURSOR) - 1 + \
     sizeof(SHOW_CURSOR) - 1 + CURSOR_POSITION_BUFFER_CAPACITY)

#define WRITE(string_literal) \
    write(STDOUT_FILENO, (string_literal), sizeof((string_literal)) - 1)

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

static void window_restore_terminal_attr(void) {
    assert(window.is_init);
    if (tcsetattr(STDOUT_FILENO, TCSANOW, &window.old_attr)) {
        log_errorf_errno(TCSETATTR_FAILED_MESSAGE);
        exit(EXIT_FAILURE);
    }
}

static void get_terminal_size(int *const restrict width,
                              int *const restrict height) NONNULL();

static void get_terminal_size(int *const restrict width,
                              int *const restrict height) {
    assert(width != NULL);
    assert(height != NULL);

    struct winsize window_size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size)) {
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

static void handle_sigcont(const int _sig) {
    (void)_sig;
    log_debugf("SIGCONT received");
    window_show_cursor();
}

void window_init(void) {
    assert(!window.is_init);

    get_terminal_size(&window.width, &window.height);

    const size_t window_size = window.width * window.height;
    window.pixels = malloc_or_exit(sizeof(*window.pixels) * window_size,
                                   "failed to create window pixels buffer");
    window.display_buffer =
        malloc_or_exit(DISPLAY_BUFFER_SIZE(window.display_buffer, window_size),
                       "failed to create window display buffer");

    if (WRITE(SWITCH_TO_ALTERNATE_SCREEN) < 0) {
        log_errorf_errno("failed to switch to alternate screen: write failed");
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(STDOUT_FILENO, &window.old_attr)) {
        log_errorf_errno(TCGETATTR_FAILED_MESSAGE);
        exit(EXIT_FAILURE);
    }

    struct termios attr;
    if (tcgetattr(STDOUT_FILENO, &attr)) {
        log_errorf_errno(TCGETATTR_FAILED_MESSAGE);
        exit(EXIT_FAILURE);
    }
    attr.c_iflag &= ~(ICRNL | IXON);
    attr.c_oflag &= ~(OPOST);
    attr.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
    attr.c_cc[VMIN] = 0;
    if (tcsetattr(STDOUT_FILENO, TCSANOW, &attr)) {
        log_errorf_errno(TCSETATTR_FAILED_MESSAGE);
        exit(EXIT_FAILURE);
    }

    window_hide_cursor();

    if (signal(SIGCONT, handle_sigcont) == SIG_ERR) {
        log_errorf_errno("failed to setup SIGCONT handler");
        exit(EXIT_FAILURE);
    }

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
    window_restore_terminal_attr();

    if (WRITE(SWITCH_TO_REGULAR_SCREEN) < 0) {
        log_errorf_errno("failed to switch to regular screen: write failed");
        exit(EXIT_FAILURE);
    }

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
        const size_t old_window_size = window.width * window.height;
        for (size_t i = 0; i < old_window_size; ++i) {
            mutex_destroy(&window.pixels[i].mutex);
        }
        const size_t new_window_size = width * height;
        window.pixels = realloc_or_exit(
            window.pixels, sizeof(*window.pixels) * new_window_size,
            "failed to resize window pixels buffer");
        for (size_t i = 0; i < new_window_size; ++i) {
            pthread_mutex_init(&window.pixels[i].mutex, NULL);
        }
        window.display_buffer = realloc_or_exit(
            window.display_buffer,
            DISPLAY_BUFFER_SIZE(window.display_buffer, new_window_size),
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
    window_detect_resize();
    handle_keyboard_input();
}

void window_clear(void) {
    assert(window.is_init);
    for (int y = 0; y < window.height; ++y) {
        const int row_offset = y * window.width;
        for (int x = 0; x < window.width; ++x) {
            window_set_pixel(row_offset + x, ' ', COLOR_WHITE, FLT_MAX);
        }
    }
}

#define ANSI_ESCAPE(color) "\033[" #color "m"

void window_flush(void) {
    assert(window.is_init);

    static const char *colors[COLOR_COUNT] = {
#define COLOR(name, code) "\033[" #code "m",
        COLORS
#undef COLOR
    };

#define DISPLAY_BUFFER_APPEND(string)                                 \
    do {                                                              \
        for (uint8_t j = 0; string[j]; ++j)                           \
            window.display_buffer[display_buffer_size++] = string[j]; \
    } while (0)

    size_t display_buffer_size = 0;
    DISPLAY_BUFFER_APPEND(HIDE_CURSOR);
    DISPLAY_BUFFER_APPEND(MOVE_CURSOR_TOP_LEFT);
    DISPLAY_BUFFER_APPEND(colors[window.pixels[0].color]);
    Color last_color = window.pixels[0].color;
    for (int y = 0; y < window.height; ++y) {
        const int row_offset = y * window.width;
        for (int x = 0; x < window.width; ++x) {
            const int index = row_offset + x;
            const Color color = window.pixels[index].color;
            if (color != last_color) {
                DISPLAY_BUFFER_APPEND(colors[color]);
                last_color = color;
            }
            window.display_buffer[display_buffer_size++] =
                window.pixels[index].chr;
        }
    }
    if (window.show_cursor) {
        char cursor_position_buffer[CURSOR_POSITION_BUFFER_CAPACITY];
        snprintf(cursor_position_buffer, CURSOR_POSITION_BUFFER_CAPACITY,
                 "\033[%d;%dH", window.cursor_position.y + 1,
                 window.cursor_position.x + 1);
        DISPLAY_BUFFER_APPEND(cursor_position_buffer);
        DISPLAY_BUFFER_APPEND(SHOW_CURSOR);
    }

    if (write(STDOUT_FILENO, window.display_buffer, display_buffer_size) < 0) {
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

static char line_get_char(const int dx, const int dy) {
    if (dx == 0) return '|';
    if (dy == 0) return '-';

    const float dir = fabsf((float)dy / dx);

    // TODO maybe put this in the config because it depends on the terminal font
    if (dir < 0.2) return '-';
    if (dir > 2.0) return '|';
    if (dx * dy < 0) return '/';

    return '\\';
}

static void window_render_line(v3f v1, v3f v2, const Color color) {
    assert(window.is_init);

    int x1 = v1.x;
    int y1 = v1.y;
    float z1 = v1.z;
    int x2 = v2.x;
    int y2 = v2.y;
    float z2 = v2.z;

    int dx = x2 - x1;
    int dy = y2 - y1;

    const char line_char = line_get_char(dx, dy);

    if (dx == 0 && dy == 0) {
        if (0 <= x1 && x1 < window.width && 0 <= y1 && y1 < window.height) {
            const int index = y1 * window.width + x1;
            const float z = fminf(z1, z2);
            mutex_lock(&window.pixels[index].mutex);
            if (z < window.pixels[index].z) {
                window.pixels[index].chr = line_char;
                window.pixels[index].color = color;
                window.pixels[index].z = z;
            }
            mutex_unlock(&window.pixels[index].mutex);
        }
        return;
    }

    if (abs(dx) > abs(dy)) {
        if (dx < 0) {
            const int tmp_x = x1;
            x1 = x2;
            x2 = tmp_x;
            dx = x2 - x1;

            const int tmp_y = y1;
            y1 = y2;
            y2 = tmp_y;
            dy = y2 - y1;

            const float tmp_z = z1;
            z1 = z2;
            z2 = tmp_z;
        }

        for (int x = x1; x <= x2; ++x) {
            const int y = dy * (x - x1) / dx + y1;
            if (0 <= x && x < window.width && 0 <= y && y < window.height) {
                const float t = (float)(x - x1) / dx;
                const float z =
                    (1.0f - t) * z1 + t * z2 - MESH_OUTLINE_Z_CORRECTION;
                const int index = y * window.width + x;
                mutex_lock(&window.pixels[index].mutex);
                if (z < window.pixels[index].z) {
                    window.pixels[index].chr = line_char;
                    window.pixels[index].color = color;
                    window.pixels[index].z = z;
                }
                mutex_unlock(&window.pixels[index].mutex);
            }
        }
    } else {
        if (dy < 0) {
            const int tmp_x = x1;
            x1 = x2;
            x2 = tmp_x;
            dx = x2 - x1;

            const int tmp_y = y1;
            y1 = y2;
            y2 = tmp_y;
            dy = y2 - y1;

            const float tmp_z = z1;
            z1 = z2;
            z2 = tmp_z;
        }

        for (int y = y1; y <= y2; ++y) {
            const int x = dx * (y - y1) / dy + x1;
            if (0 <= x && x < window.width && 0 <= y && y < window.height) {
                const float t = (float)(y - y1) / dy;
                const float z =
                    (1.0f - t) * z1 + t * z2 - MESH_OUTLINE_Z_CORRECTION;
                const int index = y * window.width + x;
                mutex_lock(&window.pixels[index].mutex);
                if (z < window.pixels[index].z) {
                    window.pixels[index].chr = line_char;
                    window.pixels[index].color = color;
                    window.pixels[index].z = z;
                }
                mutex_unlock(&window.pixels[index].mutex);
            }
        }
    }
}

#ifndef RENDER_WIREFRAME
static void window_render_triangle_fill(const Triangle3D *const triangle)
    NONNULL();

static void window_render_triangle_fill(const Triangle3D *const triangle) {
    assert(window.is_init);

    assert(triangle != NULL);

    const int x1 = triangle->v1.x;
    const int x2 = triangle->v2.x;
    const int x3 = triangle->v3.x;
    const int y1 = triangle->v1.y;
    const int y2 = triangle->v2.y;
    const int y3 = triangle->v3.y;

    const v2i v1 = {x1, y1};
    const v2i v2 = {x2, y2};
    const v2i v3 = {x3, y3};

    const int area = v2i_get_determinant(v1, v2, v3);
    if (area == 0) return;

    const int xmin = min3_float(x1, x2, x3);
    const int ymin = min3_float(y1, y2, y3);
    const int xmax = min_int(window.width - 1, max3_int(x1, x2, x3));
    const int ymax = min_int(window.height - 1, max3_int(y1, y2, y3));

    assert(xmin >= 0);
    assert(ymin >= 0);

    const float z1 = triangle->v1.z;
    const float z2 = triangle->v2.z;
    const float z3 = triangle->v3.z;

    v2i position;
    for (position.y = ymin; position.y <= ymax; ++position.y) {
        const int row_offset = position.y * window.width;
        for (position.x = xmin; position.x <= xmax; ++position.x) {
            const int w1 = v2i_get_determinant(v2, v3, position);
            const int w2 = v2i_get_determinant(v3, v1, position);
            const int w3 = v2i_get_determinant(v1, v2, position);
            if (w1 >= 0 && w2 >= 0 && w3 >= 0) {
                const float z =
                    (float)((z1 * w1) + (z2 * w2) + (z3 * w3)) / area;
                const int index = row_offset + position.x;
                mutex_lock(&window.pixels[index].mutex);
                if (z < window.pixels[index].z) {
                    window.pixels[index].chr = triangle->shade;
                    window.pixels[index].color = triangle->color;
                    window.pixels[index].z = z;
                }
                mutex_unlock(&window.pixels[index].mutex);
            }
        }
    }

    if (triangle->edges & (TRIANGLE_EDGE_V1_V2 | TRIANGLE_EDGE_V1_V2_FAR))
        window_render_line(triangle->v1, triangle->v2, MESH_OUTLINE_COLOR);

    if (triangle->edges & (TRIANGLE_EDGE_V2_V3 | TRIANGLE_EDGE_V2_V3_FAR))
        window_render_line(triangle->v2, triangle->v3, MESH_OUTLINE_COLOR);

    if (triangle->edges & (TRIANGLE_EDGE_V3_V1 | TRIANGLE_EDGE_V3_V1_FAR))
        window_render_line(triangle->v3, triangle->v1, MESH_OUTLINE_COLOR);
}
#endif

#ifdef RENDER_WIREFRAME
static void window_render_triangle_wireframe(const Triangle3D *const triangle)
    NONNULL();

static void window_render_triangle_wireframe(const Triangle3D *const triangle) {
    assert(window.is_init);
    assert(triangle != NULL);
    window_render_line(triangle->v1, triangle->v2, triangle->color);
    window_render_line(triangle->v2, triangle->v3, triangle->color);
    window_render_line(triangle->v3, triangle->v1, triangle->color);
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
