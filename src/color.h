#pragma once

#include <stdint.h>

#define COLORS              \
    COLOR(DARK_RED, 31)     \
    COLOR(DARK_GREEN, 32)   \
    COLOR(DARK_YELLOW, 33)  \
    COLOR(DARK_BLUE, 34)    \
    COLOR(DARK_MAGENTA, 35) \
    COLOR(DARK_CYAN, 36)    \
    COLOR(LIGHT_GREY, 37)   \
    COLOR(DARK_GREY, 90)    \
    COLOR(RED, 91)          \
    COLOR(GREEN, 92)        \
    COLOR(YELLOW, 93)       \
    COLOR(BLUE, 94)         \
    COLOR(MAGENTA, 95)      \
    COLOR(CYAN, 96)         \
    COLOR(WHITE, 97)

typedef enum : uint8_t {
#define COLOR(name, code) COLOR_##name,
    COLORS
#undef COLOR
        COLOR_COUNT,
} Color;
