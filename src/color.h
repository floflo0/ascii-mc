#pragma once

#include <stdint.h>

#define COLORS                  \
    COLOR(WHITE, 0)             \
    COLOR(RED, 31)              \
    COLOR(GREEN, 32)            \
    COLOR(YELLOW, 33)           \
    COLOR(BLUE, 34)             \
    COLOR(MAGENTA, 35)          \
    COLOR(CYAN, 36)             \
    COLOR(LIGHT_LIGHT_GREY, 37) \
    COLOR(DARK_GREY, 90)        \
    COLOR(LIGHT_GREY, 97)

typedef enum : uint8_t {
#define COLOR(name, code) COLOR_##name,
    COLORS
#undef COLOR
        COLOR_COUNT,
} Color;
