#include "gamepad.h"

#include <SDL3/SDL.h>
#include <stdlib.h>

#include "config.h"
#include "event_queue.h"
#include "log.h"
#include "utils.h"
#include "vec.h"

#define GAMEPAD_AXIS_MAX 32767
#define GAMEPAD_AXIS_MIN -32768

#define BUTTONS_MAPPING                                         \
    BUTTON(GAMEPAD_BUTTON_B, SDL_GAMEPAD_BUTTON_SOUTH)          \
    BUTTON(GAMEPAD_BUTTON_A, SDL_GAMEPAD_BUTTON_EAST)           \
    BUTTON(GAMEPAD_BUTTON_Y, SDL_GAMEPAD_BUTTON_WEST)           \
    BUTTON(GAMEPAD_BUTTON_X, SDL_GAMEPAD_BUTTON_NORTH)          \
    BUTTON(GAMEPAD_BUTTON_L, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)  \
    BUTTON(GAMEPAD_BUTTON_R, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) \
    BUTTON(GAMEPAD_BUTTON_MINUS, SDL_GAMEPAD_BUTTON_BACK)       \
    BUTTON(GAMEPAD_BUTTON_PLUS, SDL_GAMEPAD_BUTTON_START)       \
    BUTTON(GAMEPAD_BUTTON_HOME, SDL_GAMEPAD_BUTTON_GUIDE)       \
    BUTTON(GAMEPAD_BUTTON_LPAD, SDL_GAMEPAD_BUTTON_LEFT_STICK)  \
    BUTTON(GAMEPAD_BUTTON_RPAD, SDL_GAMEPAD_BUTTON_RIGHT_STICK) \
    BUTTON(GAMEPAD_BUTTON_UP, SDL_GAMEPAD_BUTTON_DPAD_UP)       \
    BUTTON(GAMEPAD_BUTTON_DOWN, SDL_GAMEPAD_BUTTON_DPAD_DOWN)   \
    BUTTON(GAMEPAD_BUTTON_LEFT, SDL_GAMEPAD_BUTTON_DPAD_LEFT)   \
    BUTTON(GAMEPAD_BUTTON_RIGHT, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)

struct _Gamepad {
    SDL_Gamepad *sdl_gamepad;
    bool can_rumble;
};

void gamepad_init(void) {
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        log_errorf("failed to initialized SDL gamepad subsystem: %s",
                   SDL_GetError());
        exit(EXIT_FAILURE);
    }
    assert(SDL_GamepadEventsEnabled());
    log_debugf("initialized SDL gamepad subsystem");
}

void gamepad_quit(void) {
    log_debugf("quit SDL");
    SDL_Quit();
}

#ifndef PROD
/**
 * TODO: document
 */
[[gnu::nonnull(1, 3)]]
static void gamepad_enumerate_properties(void *const restrict data,
                                            const SDL_PropertiesID props,
                                            const char *const restrict name) {
    assert(data != NULL);
    assert(name != NULL);
    Gamepad *const self = (Gamepad *)data;

    log_debugf("gamepad '%s' has property '%s'", gamepad_get_name(self), name);

    switch (SDL_GetPropertyType(props, name)) {
        case SDL_PROPERTY_TYPE_POINTER:
            log_debugf("'%s' is a pointer property", name);
            break;
        case SDL_PROPERTY_TYPE_STRING:
            log_debugf("'%s' = '%s'", name,
                       SDL_GetStringProperty(props, name, ""));
            break;
        case SDL_PROPERTY_TYPE_NUMBER:
            log_debugf("'%s' = %" SDL_PRIs64, name,
                       SDL_GetNumberProperty(props, name, 0));
            break;
        case SDL_PROPERTY_TYPE_FLOAT:
            log_debugf("'%s' = %f", name,
                       SDL_GetFloatProperty(props, name, 0.0f));
            break;
        case SDL_PROPERTY_TYPE_BOOLEAN:
            log_debugf("'%s' = %s", name,
                       BOOL_TO_STR(SDL_GetBooleanProperty(props, name, false)));
            break;
        case SDL_PROPERTY_TYPE_INVALID:
        default:
            log_debugf("'%s' is an invalid property", name);
            break;
    }
}
#endif

/**
 * TODO: document
 */
[[gnu::returns_nonnull]]
static Gamepad *gamepad_from_joystick_id(
    const SDL_JoystickID joystick_id) {
    assert(joystick_id != 0);
    assert(SDL_IsGamepad(joystick_id));
    Gamepad *self = malloc_or_exit(sizeof(*self), "failed to create gamepad");

    self->sdl_gamepad = SDL_OpenGamepad(joystick_id);
    if (self->sdl_gamepad == NULL) {
        log_errorf("failed to open sdl gamepad: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    const SDL_PropertiesID properties =
        SDL_GetGamepadProperties(self->sdl_gamepad);
    if (properties == 0) {
        log_errorf("failed to get SDL gamepad properties: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    self->can_rumble = SDL_GetBooleanProperty(
        properties, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false);
#ifndef PROD
    if (!SDL_EnumerateProperties(properties, gamepad_enumerate_properties,
                                 self)) {
        log_errorf("failed to enumerate properties for gamepad '%s'",
                   gamepad_get_name(self));
        exit(EXIT_FAILURE);
    }
#endif

    log_debugf("connect to gamepad '%s'", gamepad_get_name(self));

#ifdef GAMEPAD_RUMBLE_ONE_CONNECTION
    gamepad_rumble(self);
#endif

    return self;
}

/**
 * TODO: document
 */
static GamepadButton sdl_gamepad_button_to_gamepad_button(
    const SDL_GamepadButton button) {
    assert(button != SDL_GAMEPAD_BUTTON_INVALID);

    switch (button) {
#define BUTTON(gamepad_button, sdl_gamepad_button) \
    case sdl_gamepad_button:                       \
        return gamepad_button;

        BUTTONS_MAPPING

#undef BUTTON

        default:
            log_debugf("unexepected button '%d'", button);
            assert(false && "unreachable");
            __builtin_unreachable();
    }
}

/**
 * TODO: document
 */
static void handle_button_event(const SDL_JoystickID joystick_id,
                                const EventType event_type,
                                const GamepadButton button) {
    assert(joystick_id != 0);
    assert(event_type == EVENT_TYPE_BUTTON_DOWN ||
           event_type == EVENT_TYPE_BUTTON_UP);
    assert(button < GAMEPAD_BUTTONS_COUNT);

    const int player_index = SDL_GetGamepadPlayerIndexForID(joystick_id);
    assert(-1 <= player_index && player_index < 4);

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

void gamepad_update(void) {
    SDL_UpdateGamepads();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        assert(SDL_EVENT_JOYSTICK_ADDED == 1541);
        switch (event.type) {
#ifndef PROD
            case SDL_EVENT_JOYSTICK_AXIS_MOTION:
            case SDL_EVENT_JOYSTICK_HAT_MOTION:
            case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            case SDL_EVENT_JOYSTICK_BUTTON_UP:
            case SDL_EVENT_JOYSTICK_ADDED:
            case SDL_EVENT_JOYSTICK_UPDATE_COMPLETE:
            case SDL_EVENT_GAMEPAD_UPDATE_COMPLETE:
                break;
#endif

            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER) {
                    if (event.gaxis.value == GAMEPAD_AXIS_MAX) {
                        handle_button_event(event.gaxis.which,
                                            EVENT_TYPE_BUTTON_DOWN,
                                            GAMEPAD_BUTTON_ZL);
                    } else if (event.gaxis.value == 0) {
                        handle_button_event(event.gaxis.which,
                                            EVENT_TYPE_BUTTON_UP,
                                            GAMEPAD_BUTTON_ZL);
                    }
                } else if (event.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
                    if (event.gaxis.value == GAMEPAD_AXIS_MAX) {
                        handle_button_event(event.gaxis.which,
                                            EVENT_TYPE_BUTTON_DOWN,
                                            GAMEPAD_BUTTON_ZR);
                    } else if (event.gaxis.value == 0) {
                        handle_button_event(event.gaxis.which,
                                            EVENT_TYPE_BUTTON_UP,
                                            GAMEPAD_BUTTON_ZR);
                    }
                }
                break;

            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                handle_button_event(
                    event.gbutton.which, EVENT_TYPE_BUTTON_DOWN,
                    sdl_gamepad_button_to_gamepad_button(event.gbutton.button));
                break;

            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                handle_button_event(
                    event.gbutton.which, EVENT_TYPE_BUTTON_UP,
                    sdl_gamepad_button_to_gamepad_button(event.gbutton.button));
                break;

            case SDL_EVENT_GAMEPAD_ADDED:
                Gamepad *const gamepad =
                    gamepad_from_joystick_id(event.gdevice.which);
                event_queue_push(&(Event){
                    .type = EVENT_TYPE_GAMEPAD_CONNECT,
                    .gamepad_event =
                        {
                            .gamepad = gamepad,
                        },
                });
                break;

            default:
                log_debugf("unexepected event %u", event.type);
                assert(false && "unexepected event");
        }
    }
}

const char *gamepad_get_name(const Gamepad *const self) {
    assert(self != NULL);
    assert(self->sdl_gamepad != NULL);
    const char *const name = SDL_GetGamepadName(self->sdl_gamepad);
    if (name == NULL) return "unamed gamepad";
    return name;
}

int8_t gamepad_get_player_index(const Gamepad *const self) {
    assert(self != NULL);
    assert(self->sdl_gamepad != NULL);
    return SDL_GetGamepadPlayerIndex(self->sdl_gamepad);
}

void gamepad_set_player_index(Gamepad *const self,
                                 const int8_t player_index) {
    assert(self != NULL);
    assert(-1 <= player_index && player_index < 4);
    if (!SDL_SetGamepadPlayerIndex(self->sdl_gamepad, player_index)) {
        log_errorf("failed to set player index for gamepad '%s'",
                   gamepad_get_name(self));
        exit(EXIT_FAILURE);
    }
}

void gamepad_destroy(Gamepad *const self) {
    assert(self != NULL);
    assert(self->sdl_gamepad != NULL);
    log_debugf("disconnect gamepad '%s'", gamepad_get_name(self));
    SDL_CloseGamepad(self->sdl_gamepad);
    free(self);
}

v2f gamepad_get_stick(const Gamepad *const self, const GamepadStick stick) {
    assert(self != NULL);
    assert(self->sdl_gamepad != NULL);
    SDL_GamepadAxis axis_x;
    SDL_GamepadAxis axis_y;
    if (stick == GAMEPAD_STICK_LEFT) {
        axis_x = SDL_GAMEPAD_AXIS_LEFTX;
        axis_y = SDL_GAMEPAD_AXIS_LEFTY;
    } else if (stick == GAMEPAD_STICK_RIGHT) {
        axis_x = SDL_GAMEPAD_AXIS_RIGHTX;
        axis_y = SDL_GAMEPAD_AXIS_RIGHTY;
    } else {
        __builtin_unreachable();
    }

    const int16_t axis_x_value = SDL_GetGamepadAxis(self->sdl_gamepad, axis_x);
    const int16_t axis_y_value = SDL_GetGamepadAxis(self->sdl_gamepad, axis_y);

    v2f stick_state = {
        .x = 2.0f * (float)(axis_x_value + GAMEPAD_AXIS_MAX) /
                 (GAMEPAD_AXIS_MAX - GAMEPAD_AXIS_MIN) -
             1.0f,
        .y = 2.0f * (float)(axis_y_value + GAMEPAD_AXIS_MAX) /
                 (GAMEPAD_AXIS_MAX - GAMEPAD_AXIS_MIN) -
             1.0f,
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

/**
 * TODO: document
 */
static SDL_GamepadButton gamepad_button_to_sdl_gamepad_button(
    const GamepadButton button) {
    assert(button <= GAMEPAD_BUTTON_RIGHT);
    static const SDL_GamepadButton mapping[] = {
#define BUTTON(gamepad_button, sdl_gamepad_button) \
    [gamepad_button] = sdl_gamepad_button,
        BUTTONS_MAPPING
#undef BUTTON
    };
    return mapping[button];
}

bool gamepad_get_button(const Gamepad *const self,
                           const GamepadButton button) {
    assert(self != NULL);
    assert(self->sdl_gamepad != NULL);
    (void)button;
    if (button == GAMEPAD_BUTTON_ZL) {
        return SDL_GetGamepadAxis(self->sdl_gamepad,
                                  SDL_GAMEPAD_AXIS_LEFT_TRIGGER) ==
               GAMEPAD_AXIS_MAX;
    }
    if (button == GAMEPAD_BUTTON_ZR) {
        return SDL_GetGamepadAxis(self->sdl_gamepad,
                                  SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) ==
               GAMEPAD_AXIS_MAX;
    }
    return SDL_GetGamepadButton(self->sdl_gamepad,
                                gamepad_button_to_sdl_gamepad_button(button));
}

void gamepad_rumble(Gamepad *const self) {
    assert(self != NULL);
    assert(self->sdl_gamepad != NULL);

    if (!self->can_rumble) return;

    if (!SDL_RumbleGamepad(self->sdl_gamepad,
                           GAMEPAD_RUMBLE_EFFECT_LOW_FREQUENCY,
                           GAMEPAD_RUMBLE_EFFECT_HIGH_FREQUENCY,
                           GAMEPAD_RUMBLE_EFFECT_DURATION)) {
        log_errorf("failed to rumble the gamepad '%s': %s",
                   gamepad_get_name(self), SDL_GetError());
        exit(EXIT_FAILURE);
    }

    log_debugf("start rumble effect on gamepad '%s'",
               gamepad_get_name(self));
}
