#include "../controller.h"

#include <string.h>

#include "../config.h"
#include "../event_queue.h"
#include "../log.h"
#include "../utils.h"
#include "int_array.h"
#include "js.h"

#define STICK_LEFT_AXIS_X 0
#define STICK_LEFT_AXIS_Y 1
#define STICK_RIGHT_AXIS_X 2
#define STICK_RIGHT_AXIS_Y 3
#define ZL_AXIS 4
#define ZR_AXIS 5

#define CONTROLLER_BUTTONS                  \
    BUTTON(CONTROLLER_BUTTON_B, 0)          \
    BUTTON(CONTROLLER_BUTTON_A, 1)          \
    BUTTON(CONTROLLER_BUTTON_X, 2)          \
    BUTTON(CONTROLLER_BUTTON_Y, 3)          \
    BUTTON(CONTROLLER_BUTTON_L, 4)          \
    BUTTON(CONTROLLER_BUTTON_R, 5)          \
    TRIGGER_BUTTON(CONTROLLER_BUTTON_ZL, 6) \
    TRIGGER_BUTTON(CONTROLLER_BUTTON_ZR, 7) \
    BUTTON(CONTROLLER_BUTTON_MINUS, 8)      \
    BUTTON(CONTROLLER_BUTTON_PLUS, 9)       \
    BUTTON(CONTROLLER_BUTTON_HOME, 16)      \
    BUTTON(CONTROLLER_BUTTON_LPAD, 10)      \
    BUTTON(CONTROLLER_BUTTON_RPAD, 11)      \
    BUTTON(CONTROLLER_BUTTON_UP, 12)        \
    BUTTON(CONTROLLER_BUTTON_DOWN, 13)      \
    BUTTON(CONTROLLER_BUTTON_LEFT, 14)      \
    BUTTON(CONTROLLER_BUTTON_RIGHT, 15)

struct _Controller {
    int gamepad_index;
    int update_request_id;
    int8_t player_index;
    bool triggers_as_buttons;
    bool button_states[CONTROLLER_BUTTONS_COUNT];
};

static bool monitor_started = false;
static ControllerArray *controllers_array = NULL;
static JS_GamepadArray gamepads_array = JS_NULL;
static IntArray *gamepad_indexes_array = NULL;
static int monitor_gamepads_request_id = -1;

[[gnu::nonnull(1)]]
static void controller_push_button_event(const Controller *const self,
                                         const EventType event_type,
                                         const ControllerButton button) {
    assert(self != NULL);
    assert(event_type == EVENT_TYPE_BUTTON_DOWN ||
           event_type == EVENT_TYPE_BUTTON_UP);
    assert(button < CONTROLLER_BUTTONS_COUNT);

    if (self->player_index == -1) return;

    event_queue_push(&(Event){
        .type = event_type,
        .button_event =
            {
                .player_index = self->player_index,
                .button = button,
            },
    });
}

[[gnu::nonnull(1)]]
static void controller_handle_button_down(Controller *const self,
                                          const ControllerButton button) {
    assert(self != NULL);
    assert(button < CONTROLLER_BUTTONS_COUNT);
    self->button_states[button] = true;
    controller_push_button_event(self, EVENT_TYPE_BUTTON_DOWN, button);
}

[[gnu::nonnull(1)]]
static void controller_handle_button_up(Controller *const self,
                                        const ControllerButton button) {
    assert(self != NULL);
    assert(button < CONTROLLER_BUTTONS_COUNT);
    self->button_states[button] = false;
    controller_push_button_event(self, EVENT_TYPE_BUTTON_UP, button);
}

static void controller_update(void *const data) {
    assert(data != NULL);
    assert(gamepads_array != JS_NULL);
    Controller *const self = data;
    const int gamepad_index = self->gamepad_index;
    const JS_Gamepad gamepad =
        JS_GamepadArray_get(gamepads_array, gamepad_index);

    if (gamepad == JS_NULL || !JS_Gamepad_get_connected(gamepad)) {
        event_queue_push(&(Event){
            .type = EVENT_TYPE_CONTROLLER_DISCONNECT,
            .controller_event = {.controller = self},
        });
        self->update_request_id = -1;
        for (size_t i = 0; i < gamepad_indexes_array->length; ++i) {
            if (gamepad_indexes_array->array[i] == gamepad_index) {
                int_array_remove(gamepad_indexes_array, i);
                break;
            }
        }
        JS_Object_free(gamepad);
        return;
    }

#define BUTTON(controller_button, button_index)                         \
    {                                                                   \
        const bool button_state =                                       \
            JS_Gamepad_get_button(gamepad, button_index);               \
        if (self->button_states[controller_button] != button_state) {   \
            if (button_state) {                                         \
                controller_handle_button_down(self, controller_button); \
            } else {                                                    \
                controller_handle_button_up(self, controller_button);   \
            }                                                           \
        }                                                               \
    }

#define TRIGGER_BUTTON(controller_button, button_index) \
    if (self->triggers_as_buttons) BUTTON(controller_button, button_index);

    CONTROLLER_BUTTONS

#undef BUTTON

    if (!self->triggers_as_buttons) {
        const float zl_axis = JS_Gamepad_get_axe(gamepad, ZL_AXIS);
        const bool zl_state = zl_axis > 0;
        if (self->button_states[CONTROLLER_BUTTON_ZL] != zl_state) {
            if (zl_state) {
                controller_handle_button_down(self, CONTROLLER_BUTTON_ZL);
            } else {
                controller_handle_button_up(self, CONTROLLER_BUTTON_ZL);
            }
        }

        const float zr_axis = JS_Gamepad_get_axe(gamepad, ZR_AXIS);
        const bool zr_state = zr_axis > 0;
        if (self->button_states[CONTROLLER_BUTTON_ZR] != zr_state) {
            if (zr_state) {
                controller_handle_button_down(self, CONTROLLER_BUTTON_ZR);
            } else {
                controller_handle_button_up(self, CONTROLLER_BUTTON_ZR);
            }
        }
    }

    JS_Object_free(gamepad);

    self->update_request_id =
        JS_requestAnimationFrame_data(controller_update, data);
}

[[gnu::returns_nonnull]]
static Controller *controller_create(const JS_Gamepad gamepad) {
    Controller *const self =
        malloc_or_exit(sizeof(*self), "failed to create controller");

    self->gamepad_index = JS_Gamepad_get_index(gamepad);
    int_array_push(gamepad_indexes_array, self->gamepad_index);
    self->update_request_id = -1;
    self->triggers_as_buttons = JS_Gamepad_get_axes_length(gamepad) == 4;
    memset(self->button_states, false, sizeof(self->button_states));

    if (monitor_started) controller_update(self);

    controller_rumble(self);

    log_debugf("connect to controller '%s'", controller_get_name(self));

    return self;
}

static void controller_connect_if_not_null(const JS_Gamepad gamepad) {
    assert(controllers_array != NULL);
    if (gamepad == JS_NULL) return;
    controller_array_push(controllers_array, controller_create(gamepad));
    JS_Object_free(gamepad);
}

ControllerArray *controller_get_connected_controllers(void) {
    if (gamepad_indexes_array == NULL) {
        gamepad_indexes_array = int_array_create(1);
    }
    if (controllers_array == NULL) {
        controllers_array = controller_array_create(1);
        gamepads_array = JS_navigator_getGamepads();
        JS_Array_foreach(gamepads_array, controller_connect_if_not_null);
    }

    return controllers_array;
}

static void controller_connect_if_new(const JS_Gamepad gamepad) {
    assert(controllers_array != NULL);
    if (gamepad == JS_NULL) return;
    if (!JS_Gamepad_get_connected(gamepad)) {
        goto return_;
    }
    const int index = JS_Gamepad_get_index(gamepad);
    for (size_t i = 0; i < gamepad_indexes_array->length; ++i) {
        if (index == gamepad_indexes_array->array[i]) goto return_;
    }

    event_queue_push(&(Event){
        .type = EVENT_TYPE_CONTROLLER_CONNECT,
        .controller_event =
            {
                .controller = controller_create(gamepad),
            },
    });

return_:
    JS_Object_free(gamepad);
    return;
}

static void controller_monitor_gamepads(void) {
    if (gamepads_array != JS_NULL) JS_Object_free(gamepads_array);
    gamepads_array = JS_navigator_getGamepads();
    assert(gamepads_array != JS_NULL);
    JS_Array_foreach(gamepads_array, controller_connect_if_new);
    monitor_gamepads_request_id =
        JS_requestAnimationFrame(controller_monitor_gamepads);
}

void controller_start_monitor(void) {
    monitor_started = true;
    for (size_t i = 0; i < controllers_array->length; ++i) {
        controller_update(controllers_array->array[i]);
    }
    controller_monitor_gamepads();
}

void controller_stop_monitor(void) {
    if (monitor_gamepads_request_id == -1) return;

    JS_cancelAnimationFrame(monitor_gamepads_request_id);
    assert(gamepads_array != JS_NULL);
    JS_Object_free(gamepads_array);
    gamepads_array = JS_NULL;
    array_destroy((Array *)gamepad_indexes_array);
    gamepad_indexes_array = NULL;
    monitor_started = false;
}

#ifndef PROD
const char *controller_get_name(const Controller *const self) {
    assert(self != NULL);
    static char controller_id[64];
    JS_GamepadArray array = gamepads_array;
    if (gamepads_array == JS_NULL) array = JS_navigator_getGamepads();
    const JS_Gamepad gamepad = JS_GamepadArray_get(array, self->gamepad_index);
    assert(gamepad != JS_NULL);
    JS_Gamepad_get_id(gamepad, controller_id, sizeof(controller_id));
    JS_Object_free(gamepad);
    if (gamepads_array == JS_NULL) JS_Object_free(array);
    return controller_id;
}
#endif

int8_t controller_get_player_index(const Controller *const self) {
    assert(self != NULL);
    return self->player_index;
}

void controller_set_player_index(Controller *const self,
                                 const int8_t player_index) {
    assert(self != NULL);
    assert(-1 <= player_index && player_index < 4);
    self->player_index = player_index;
}

void controller_destroy(Controller *const self) {
    assert(self != NULL);
    log_debugf("disconnect controller '%s'", controller_get_name(self));
    if (self->update_request_id != -1)
        JS_cancelAnimationFrame(self->update_request_id);
    free(self);
}

v2f controller_get_stick(const Controller *const self,
                         const ControllerStick stick) {
    assert(self != NULL);
    assert(self->gamepad_index != JS_NULL);
    assert(gamepads_array != JS_NULL);
    const JS_Gamepad gamepad =
        JS_GamepadArray_get(gamepads_array, self->gamepad_index);
    assert(gamepad != JS_NULL);
    int axis_x, axis_y;
    switch (stick) {
        case CONTROLLER_STICK_LEFT:
            axis_x = STICK_LEFT_AXIS_X;
            axis_y = STICK_LEFT_AXIS_Y;
            break;

        case CONTROLLER_STICK_RIGHT:
            axis_x = STICK_RIGHT_AXIS_X;
            axis_y = STICK_RIGHT_AXIS_Y;
            break;
    }

    v2f stick_state = (v2f){
        .x = JS_Gamepad_get_axe(gamepad, axis_x),
        .y = JS_Gamepad_get_axe(gamepad, axis_y),
    };

    if (fabsf(stick_state.x) < CONTROLLER_AXIS_ROUND) {
        stick_state.x = 0.0f;
    } else if (stick_state.x > 1.0f - CONTROLLER_AXIS_ROUND) {
        stick_state.x = 1.0f;
    } else if (stick_state.x < -1.0f + CONTROLLER_AXIS_ROUND) {
        stick_state.x = -1.0f;
    }
    if (fabsf(stick_state.y) < CONTROLLER_AXIS_ROUND) {
        stick_state.y = 0.0f;
    } else if (stick_state.y > 1.0f - CONTROLLER_AXIS_ROUND) {
        stick_state.y = 1.0f;
    } else if (stick_state.y < -1.0f + CONTROLLER_AXIS_ROUND) {
        stick_state.y = -1.0f;
    }

    const float stick_state_norm = v2f_norm(stick_state);
    if (stick_state_norm > 1.0f) {
        stick_state = v2f_div(stick_state, stick_state_norm);
    }

    JS_Object_free(gamepad);

    return stick_state;
}

bool controller_get_button(const Controller *const self,
                           const ControllerButton button) {
    assert(self != NULL);
    assert(button < CONTROLLER_BUTTONS_COUNT);
    return self->button_states[button];
}

// TODO: controller_rumble implementation
bool controller_rumble([[maybe_unused]] Controller *const self) {
    assert(self != NULL);
    return true;
}
