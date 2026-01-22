#pragma once

#include <stdbool.h>

#include "gamepad_defs.h"
#include "vec_defs.h"

/**
 * TODO: document
 */
void gamepad_init(void);

/**
 * TODO: document
 */
void gamepad_quit(void);

/**
 * TODO: document
 */
void gamepad_update(void);

/**
 * TODO: document
 */
[[gnu::nonnull(1)]]
const char *gamepad_get_name(const Gamepad *const self);

/**
 * TODO: document
 */
[[gnu::nonnull]]
int8_t gamepad_get_player_index(const Gamepad *const self);

/**
 * TODO: document
 */
[[gnu::nonnull(1)]]
void gamepad_set_player_index(Gamepad *const self, const int8_t player_index);

/**
 * Destroy a gamepad structure.
 *
 * \param self The pointer of the gamepad structure to destroy.
 */
[[gnu::nonnull]]
void gamepad_destroy(Gamepad *const self);

/**
 * Retrieves the current position of the specified analog stick (left or right)
 * on the gamepad.
 *
 * The function normalizes the raw axis values of the stick to a floating-point
 * range between -1.0 and 1.0, where -1.0 represents the minimum position, 1.0
 * represents the maximum position, and 0.0 represents the center.
 *
 * \param self A pointer to the gamepad structure from which to retrieve the
 *             stick position.
 * \param stick The stick to retrieve the position for. This should be either
 *              GAMEPAD_STICK_LEFT or GAMEPAD_STICK_RIGHT.
 *
 * \returns a vector containing the stick state.
 */
[[gnu::nonnull(1)]]
v2f gamepad_get_stick(const Gamepad *const self, const GamepadStick stick);

/**
 * Retrieves the current state of a button on a gamepad.
 *
 * \param self A pointer to the gamepad structure from which to retrieve the
 *             button state.
 * \param button The button to get the state.
 *
 * \returns true if the button is currently pressed, false otherwise.
 */
[[gnu::nonnull(1)]]
bool gamepad_get_button(const Gamepad *const self, const GamepadButton button);

/**
 * Start a rumble effect on the gamepad.
 *
 * \param self A pointer to the gamepad object on which to activate the
 *             rumble effect.
 */
[[gnu::nonnull]]
void gamepad_rumble(Gamepad *const self);
