#pragma once

#include <libevdev/libevdev.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "array.h"
#include "vec.h"

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define RETURNS_NONNULL __attribute__((returns_nonnull))

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

typedef struct {
    struct libevdev *dev;
    pthread_t update_thread;
    pthread_t rumble_thread;
    pthread_mutex_t rumble_mutex;
    int fd;
    int16_t rumble_effect_id;
    int8_t player_index;
    bool is_rumbling;
    bool button_states[CONTROLLER_BUTTONS_COUNT];
} Controller;

typedef enum : uint8_t {
    CONTROLLER_STICK_LEFT,
    CONTROLLER_STICK_RIGHT
} ControllerStick;

DEFINE_ARRAY(controller, Controller, Controller *)

void controller_array_destroy(ControllerArray *array) NONNULL();

ControllerArray *controller_get_connected_controllers(void) RETURNS_NONNULL;

void controller_start_monitor_thread(void);

void controller_stop_monitor_thread(void);

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
