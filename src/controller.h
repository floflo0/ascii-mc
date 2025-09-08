#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "controller_array.h"
#include "vec.h"

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define RETURNS_NONNULL __attribute__((returns_nonnull))

ControllerArray *controller_get_connected_controllers(void) RETURNS_NONNULL;

void controller_start_monitor(void);

void controller_stop_monitor(void);

#ifndef PROD
const char *controller_get_name(const Controller *const self) NONNULL(1);
#endif

int8_t controller_get_player_index(const Controller *const self) NONNULL();

void controller_set_player_index(Controller *const self,
                                 const int8_t player_index) NONNULL(1);

/**
 * Destroy a controller created by controller_from_device_path() and
 * controller_from_first().
 *
 * \param self The pointer of the controller object to destroy.
 */
void controller_destroy(Controller *const self) NONNULL();

/**
 * Retrieves the current position of the specified analog stick (left or right)
 * on the controller.
 *
 * The function normalizes the raw axis values of the stick to a floating-point
 * range between -1.0 and 1.0, where -1.0 represents the minimum position, 1.0
 * represents the maximum position, and 0.0 represents the center.
 *
 * \param self A pointer to the controller object from which to retrieve the
 *             stick position.
 * \param stick The stick to retrieve the position for. This should be either
 *              CONTROLLER_STICK_LEFT or CONTROLLER_STICK_RIGHT.
 *
 * \returns a vector containing the stick state.
 */
v2f controller_get_stick(const Controller *const self,
                         const ControllerStick stick) NONNULL(1);

bool controller_get_button(const Controller *const self,
                           const ControllerButton button) NONNULL(1);

/**
 * Start a rumble effect on the controller.
 *
 * \param self A pointer to the controller object on which to activate the
 *             rumble effect.
 *
 * \returns true on success, or false if there is an error starting the rumble.
 */
bool controller_rumble(Controller *const self) NONNULL();
