#pragma once

#include <stdint.h>

/**
 * Enum representing the buttons available on a controller.
 */
typedef enum : uint8_t {
    // Real buttons
    CONTROLLER_BUTTON_B = 0,
    CONTROLLER_BUTTON_A = 1,
    CONTROLLER_BUTTON_Y = 2,
    CONTROLLER_BUTTON_X = 3,
    CONTROLLER_BUTTON_L = 4,
    CONTROLLER_BUTTON_R = 5,
    CONTROLLER_BUTTON_MINUS = 6,
    CONTROLLER_BUTTON_PLUS = 7,
    CONTROLLER_BUTTON_HOME = 8,
    CONTROLLER_BUTTON_LPAD = 9,
    CONTROLLER_BUTTON_RPAD = 10,

    // Hat buttons
    CONTROLLER_BUTTON_UP = 11,
    CONTROLLER_BUTTON_DOWN = 12,
    CONTROLLER_BUTTON_LEFT = 13,
    CONTROLLER_BUTTON_RIGHT = 14,

    // Trigger buttns
    CONTROLLER_BUTTON_ZL = 15,
    CONTROLLER_BUTTON_ZR = 16,

    CONTROLLER_BUTTONS_COUNT,
} ControllerButton;

typedef struct _Controller Controller;

typedef enum : uint8_t {
    CONTROLLER_STICK_LEFT,
    CONTROLLER_STICK_RIGHT
} ControllerStick;
