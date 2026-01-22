#pragma once

/**
 * Types definition for the gamepad module.
 */

#include <stdint.h>

/**
 * Structure representing a gamepad instance.
 *
 * This structure stores the gamepad's internal state. The implementation is
 * hidden, as it depends on the platform.
 */
typedef struct _Gamepad Gamepad;

/**
 * Enum representing the buttons available on a gamepad.
 */
typedef enum : uint8_t {
    // Real buttons
    GAMEPAD_BUTTON_B = 0,
    GAMEPAD_BUTTON_A = 1,
    GAMEPAD_BUTTON_Y = 2,
    GAMEPAD_BUTTON_X = 3,
    GAMEPAD_BUTTON_L = 4,
    GAMEPAD_BUTTON_R = 5,
    GAMEPAD_BUTTON_MINUS = 6,
    GAMEPAD_BUTTON_PLUS = 7,
    GAMEPAD_BUTTON_HOME = 8,
    GAMEPAD_BUTTON_LPAD = 9,
    GAMEPAD_BUTTON_RPAD = 10,
    GAMEPAD_BUTTON_UP = 11,
    GAMEPAD_BUTTON_DOWN = 12,
    GAMEPAD_BUTTON_LEFT = 13,
    GAMEPAD_BUTTON_RIGHT = 14,

    // Trigger buttons
    GAMEPAD_BUTTON_ZL = 15,
    GAMEPAD_BUTTON_ZR = 16,

    GAMEPAD_BUTTONS_COUNT,
} GamepadButton;

/**
 * Enum representing the sticks available on a gamepad.
 */
typedef enum : uint8_t { GAMEPAD_STICK_LEFT, GAMEPAD_STICK_RIGHT } GamepadStick;
