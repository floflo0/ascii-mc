#include "../gamepad.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "../event_queue.h"
#include "../gamepad_array.h"
#include "../log.h"
#include "../utils.h"
#include "../vec.h"
#include "js.h"

typedef enum : uint8_t {
    BUTTON_MAPPING_TYPE_BUTTON,
    BUTTON_MAPPING_TYPE_TRIGGER,
} ButtonMappingType;

typedef struct {
    uint8_t button_index;
} ButtonMappingButton;

typedef struct {
    uint8_t trigger_axis_index;
} ButtonMappingTrigger;

typedef struct {
    ButtonMappingType type;
    union {
        ButtonMappingButton button;
        ButtonMappingTrigger trigger;
    };
} ButtonMapping;

typedef struct {
    uint8_t axis_x;
    uint8_t axis_y;
} StickMapping;

typedef struct {
    ButtonMapping button_mapping[GAMEPAD_BUTTONS_COUNT];
    StickMapping stick_mapping[2];
    size_t min_button_count;
    size_t min_axis_count;
    const char *gamepad_names[];
} Mapping;

struct _Gamepad {
    uint32_t index;
    const Mapping *mappings;
    StickMapping stick_mapping[2];
    int8_t player_index;
    bool button_states[GAMEPAD_BUTTONS_COUNT];
    bool connected;
};

#define MAP_BUTTON(mapped_button_index)              \
    {                                                \
        .type = BUTTON_MAPPING_TYPE_BUTTON,          \
        .button =                                    \
            {                                        \
                .button_index = mapped_button_index, \
            },                                       \
    }

#define MAP_TRIGGER(mapped_axis_index)                   \
    {                                                    \
        .type = BUTTON_MAPPING_TYPE_TRIGGER,             \
        .trigger =                                       \
            {                                            \
                .trigger_axis_index = mapped_axis_index, \
            },                                           \
    }

static const Mapping xbox_wireless_controller_chromium_mapping = {
    .button_mapping =
        {
            [GAMEPAD_BUTTON_B] = MAP_BUTTON(0),
            [GAMEPAD_BUTTON_A] = MAP_BUTTON(1),
            [GAMEPAD_BUTTON_Y] = MAP_BUTTON(2),
            [GAMEPAD_BUTTON_X] = MAP_BUTTON(3),
            [GAMEPAD_BUTTON_L] = MAP_BUTTON(4),
            [GAMEPAD_BUTTON_R] = MAP_BUTTON(5),
            [GAMEPAD_BUTTON_MINUS] = MAP_BUTTON(8),
            [GAMEPAD_BUTTON_PLUS] = MAP_BUTTON(9),
            [GAMEPAD_BUTTON_HOME] = MAP_BUTTON(16),
            [GAMEPAD_BUTTON_LPAD] = MAP_BUTTON(10),
            [GAMEPAD_BUTTON_RPAD] = MAP_BUTTON(11),
            [GAMEPAD_BUTTON_UP] = MAP_BUTTON(12),
            [GAMEPAD_BUTTON_DOWN] = MAP_BUTTON(13),
            [GAMEPAD_BUTTON_LEFT] = MAP_BUTTON(14),
            [GAMEPAD_BUTTON_RIGHT] = MAP_BUTTON(15),
            [GAMEPAD_BUTTON_ZL] = MAP_BUTTON(6),
            [GAMEPAD_BUTTON_ZR] = MAP_BUTTON(7),
        },
    .stick_mapping =
        {
            [GAMEPAD_STICK_LEFT] =
                {
                    .axis_x = 0,
                    .axis_y = 1,
                },
            [GAMEPAD_STICK_RIGHT] =
                {
                    .axis_x = 2,
                    .axis_y = 3,
                },
        },
    .min_button_count = 17,
    .min_axis_count = 4,
    .gamepad_names =
        {
            "Xbox Wireless Controller (STANDARD GAMEPAD Vendor: 045e Product: "
            "02fd)",
            "Gamepad Controller (STANDARD GAMEPAD Vendor: 045e Product: 028e)",
            "057e-2009-Nintendo Co., Ltd. Pro Controller",
            NULL,
        },
};

static const Mapping xbox_wireless_controller_firefox_mapping = {
    .button_mapping =
        {
            [GAMEPAD_BUTTON_B] = MAP_BUTTON(0),
            [GAMEPAD_BUTTON_A] = MAP_BUTTON(1),
            [GAMEPAD_BUTTON_Y] = MAP_BUTTON(3),
            [GAMEPAD_BUTTON_X] = MAP_BUTTON(2),
            [GAMEPAD_BUTTON_L] = MAP_BUTTON(4),
            [GAMEPAD_BUTTON_R] = MAP_BUTTON(5),
            [GAMEPAD_BUTTON_MINUS] = MAP_BUTTON(17),
            [GAMEPAD_BUTTON_PLUS] = MAP_BUTTON(9),
            [GAMEPAD_BUTTON_HOME] = MAP_BUTTON(18),
            [GAMEPAD_BUTTON_LPAD] = MAP_BUTTON(10),
            [GAMEPAD_BUTTON_RPAD] = MAP_BUTTON(11),
            [GAMEPAD_BUTTON_UP] = MAP_BUTTON(12),
            [GAMEPAD_BUTTON_DOWN] = MAP_BUTTON(13),
            [GAMEPAD_BUTTON_LEFT] = MAP_BUTTON(14),
            [GAMEPAD_BUTTON_RIGHT] = MAP_BUTTON(15),
            [GAMEPAD_BUTTON_ZL] = MAP_TRIGGER(7),
            [GAMEPAD_BUTTON_ZR] = MAP_TRIGGER(6),
        },
    .stick_mapping =
        {
            [GAMEPAD_STICK_LEFT] =
                {
                    .axis_x = 0,
                    .axis_y = 1,
                },
            [GAMEPAD_STICK_RIGHT] =
                {
                    .axis_x = 4,
                    .axis_y = 5,
                },
        },
    .min_button_count = 19,
    .min_axis_count = 8,
    .gamepad_names =
        {
            "045e-02fd-Xbox Wireless Controller",
            NULL,
        },
};

static const Mapping xbox_360_pad_firefox_mapping = {
    .button_mapping =
        {
            [GAMEPAD_BUTTON_B] = MAP_BUTTON(0),
            [GAMEPAD_BUTTON_A] = MAP_BUTTON(1),
            [GAMEPAD_BUTTON_Y] = MAP_BUTTON(3),
            [GAMEPAD_BUTTON_X] = MAP_BUTTON(2),
            [GAMEPAD_BUTTON_L] = MAP_BUTTON(4),
            [GAMEPAD_BUTTON_R] = MAP_BUTTON(5),
            [GAMEPAD_BUTTON_MINUS] = MAP_BUTTON(8),
            [GAMEPAD_BUTTON_PLUS] = MAP_BUTTON(9),
            [GAMEPAD_BUTTON_HOME] = MAP_BUTTON(16),
            [GAMEPAD_BUTTON_LPAD] = MAP_BUTTON(10),
            [GAMEPAD_BUTTON_RPAD] = MAP_BUTTON(11),
            [GAMEPAD_BUTTON_UP] = MAP_BUTTON(12),
            [GAMEPAD_BUTTON_DOWN] = MAP_BUTTON(13),
            [GAMEPAD_BUTTON_LEFT] = MAP_BUTTON(14),
            [GAMEPAD_BUTTON_RIGHT] = MAP_BUTTON(15),
            [GAMEPAD_BUTTON_ZL] = MAP_TRIGGER(4),
            [GAMEPAD_BUTTON_ZR] = MAP_TRIGGER(5),
        },
    .stick_mapping =
        {
            [GAMEPAD_STICK_LEFT] =
                {
                    .axis_x = 0,
                    .axis_y = 1,
                },
            [GAMEPAD_STICK_RIGHT] =
                {
                    .axis_x = 2,
                    .axis_y = 3,
                },
        },
    .min_button_count = 17,
    .min_axis_count = 6,
    .gamepad_names =
        {
            "045e-028e-Microsoft X-Box 360 pad",
            NULL,
        },
};

static const Mapping *mappings[] = {
    &xbox_wireless_controller_chromium_mapping,
    &xbox_wireless_controller_firefox_mapping,
    &xbox_360_pad_firefox_mapping,
};
static const Mapping *const default_mapping =
    &xbox_wireless_controller_chromium_mapping;
static GamepadArray gamepad_array;

/**
 * TODO: document
 */
static Gamepad *gamepad_from_index(const uint32_t index) {
    Gamepad *const self =
        malloc_or_exit(sizeof(*self), "failed to create gamepad");
    self->index = index;
    self->player_index = -1;
    for (GamepadButton i = 0; i < GAMEPAD_BUTTONS_COUNT; ++i) {
        self->button_states[i] = false;
    }
    self->connected = true;

    self->mappings = NULL;
    const char *const gamepad_name = gamepad_get_name(self);
    const size_t buttons_count = JS_get_gamepad_buttons_count(index);
    const size_t axis_count = JS_get_gamepad_axis_count(index);
    for (size_t i = 0;
         i < sizeof(mappings) / sizeof(*mappings) && self->mappings == NULL;
         ++i) {
        for (const char *const *name = mappings[i]->gamepad_names; *name;
             ++name) {
            if (strcmp(gamepad_name, *name) == 0) {
                if (buttons_count >= mappings[i]->min_button_count &&
                    axis_count >= mappings[i]->min_axis_count) {
                    self->mappings = mappings[i];
                }
                break;
            }
        }
    }
    if (self->mappings == NULL) {
        if (buttons_count >= default_mapping->min_button_count &&
            axis_count >= default_mapping->min_axis_count) {
            self->mappings = default_mapping;
        } else {
            log_errorf("no mapping found for controller '%s'", gamepad_name);
            free(self);
            return NULL;
        }
    }

    log_debugf("connect to gamepad '%s'", gamepad_get_name(self));

#ifdef GAMEPAD_RUMBLE_ON_CONNECTION
    gamepad_rumble(self);
#endif

    return self;
}

/**
 * TODO: document
 */
static void handle_gamepad_connected(const uint32_t index) {
    Gamepad *const gamepad = gamepad_from_index(index);
    if (gamepad == NULL) return;
    gamepad_array_push(&gamepad_array, gamepad);
    event_queue_push(&(Event){
        .type = EVENT_TYPE_GAMEPAD_CONNECT,
        .gamepad_event =
            {
                .gamepad = gamepad,
            },
    });
}

/**
 * TODO: document
 */
static void handle_gamepad_disconnected(const uint32_t index) {
    for (size_t i = 0; i < gamepad_array.length; ++i) {
        Gamepad *const gamepad = gamepad_array.array[i];
        if (gamepad->index == index) {
            gamepad->connected = false;
            event_queue_push(&(Event){
                .type = EVENT_TYPE_GAMEPAD_DISCONNECT,
                .gamepad_event =
                    {
                        .gamepad = gamepad,
                    },
            });
        }
    }
}

void gamepad_init(void) {
    gamepad_array_init(&gamepad_array, GAMEPAD_ARRAY_DEFAULT_CAPACITY);
    JS_ongamepadconnected(handle_gamepad_connected);
    JS_ongamepaddisconnected(handle_gamepad_disconnected);
}

void gamepad_quit(void) {
    assert(gamepad_array.length == 0);
    JS_ongamepadconnected(NULL);
    JS_ongamepaddisconnected(NULL);
    gamepad_array_destroy(&gamepad_array);
}

/**
 * TODO: document
 */
[[gnu::nonnull(1)]]
static void gamepad_handle_button_event(const Gamepad *const self,
                                        const EventType event_type,
                                        const GamepadButton button) {
    assert(self != NULL);
    assert(event_type == EVENT_TYPE_BUTTON_DOWN ||
           event_type == EVENT_TYPE_BUTTON_UP);
    assert(button < GAMEPAD_BUTTONS_COUNT);

    const int8_t player_index = self->player_index;

    // If the gamepad isn't assign to any player, the game don't need to process
    // this event.
    if (player_index == -1) return;

    event_queue_push(&(Event){
        .type = event_type,
        .button_event =
            {
                .player_index = player_index,
                .button = button,
            },
    });
}

/**
 * TODO: document
 */
[[gnu::nonnull(1)]]
static inline bool gamepad_get_button_state(const Gamepad *const self,
                                            const GamepadButton button) {
    assert(self != NULL);
    assert(button < GAMEPAD_BUTTONS_COUNT);

    const ButtonMapping *const mapping =
        &self->mappings->button_mapping[button];
    if (mapping->type == BUTTON_MAPPING_TYPE_BUTTON) {
        return JS_get_gamepad_button(self->index, mapping->button.button_index);
    }

    if (mapping->type == BUTTON_MAPPING_TYPE_TRIGGER) {
        return JS_Gamepad_get_axis(self->index,
                                   mapping->trigger.trigger_axis_index) == 1.0f;
    }

    assert(false && "unreachable");
    __builtin_unreachable();
}

void gamepad_update(void) {
    for (size_t i = 0; i < gamepad_array.length; ++i) {
        Gamepad *const gamepad = gamepad_array.array[i];
        assert(gamepad != NULL);
        if (!gamepad->connected) continue;
        for (GamepadButton button = 0; button < GAMEPAD_BUTTONS_COUNT;
             ++button) {
            const bool new_button_state =
                gamepad_get_button_state(gamepad, button);
            if (new_button_state != gamepad->button_states[button]) {
                gamepad->button_states[button] = new_button_state;
                if (new_button_state) {
                    gamepad_handle_button_event(gamepad, EVENT_TYPE_BUTTON_DOWN,
                                                button);
                } else {
                    gamepad_handle_button_event(gamepad, EVENT_TYPE_BUTTON_UP,
                                                button);
                }
            }
        }
    }
}

const char *gamepad_get_name(const Gamepad *const self) {
    assert(self != NULL);
    static char gamepad_id[128];
    JS_get_gamepad_id(self->index, gamepad_id, sizeof(gamepad_id));
    return gamepad_id;
}

inline int8_t gamepad_get_player_index(const Gamepad *const self) {
    assert(self != NULL);
    return self->player_index;
}

inline void gamepad_set_player_index(Gamepad *const self,
                                     const int8_t player_index) {
    assert(self != NULL);
    assert(-1 <= player_index && player_index < 4);
    self->player_index = player_index;
}

void gamepad_destroy(Gamepad *const self) {
    assert(self != NULL);
    for (size_t i = 0; i < gamepad_array.length; ++i) {
        if (gamepad_array.array[i] == self) {
            gamepad_array_remove(&gamepad_array, i);
            break;
        }
    }
    log_debugf("disconnect gamepad '%s'", gamepad_get_name(self));
    free(self);
}

v2f gamepad_get_stick(const Gamepad *const self, const GamepadStick stick) {
    assert(self != NULL);
    (void)stick;
    const StickMapping mapping = self->mappings->stick_mapping[stick];
    v2f stick_state = {
        .x = JS_Gamepad_get_axis(self->index, mapping.axis_x),
        .y = JS_Gamepad_get_axis(self->index, mapping.axis_y),
    };

    if (fabsf(stick_state.x) < GAMEPAD_AXIS_ROUND) {
        stick_state.x = 0.0f;
    } else if (stick_state.x > 1.0f - GAMEPAD_AXIS_ROUND) {
        stick_state.x = 1.0f;
    } else if (stick_state.x < -1.0f + GAMEPAD_AXIS_ROUND) {
        stick_state.x = -1.0f;
    }
    if (fabsf(stick_state.y) < GAMEPAD_AXIS_ROUND) {
        stick_state.y = 0.0f;
    } else if (stick_state.y > 1.0f - GAMEPAD_AXIS_ROUND) {
        stick_state.y = 1.0f;
    } else if (stick_state.y < -1.0f + GAMEPAD_AXIS_ROUND) {
        stick_state.y = -1.0f;
    }

    const float stick_state_norm = v2f_norm(stick_state);
    if (stick_state_norm > 1.0f) {
        stick_state = v2f_div(stick_state, stick_state_norm);
    }

    return stick_state;
}

bool gamepad_get_button(const Gamepad *const self, const GamepadButton button) {
    assert(self != NULL);
    assert(button < GAMEPAD_BUTTONS_COUNT);
    return self->button_states[button];
}

void gamepad_rumble(Gamepad *const self) {
    assert(self != NULL);
    log_debugf("try to start rumble effect on gamepad '%s'",
               gamepad_get_name(self));
    JS_gameapad_rumble(self->index, GAMEPAD_RUMBLE_EFFECT_LOW_FREQUENCY,
                       GAMEPAD_RUMBLE_EFFECT_HIGH_FREQUENCY,
                       GAMEPAD_RUMBLE_EFFECT_DURATION);
}
