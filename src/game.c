#include "game.h"

#include <assert.h>
#ifndef __wasm__
#include <signal.h>
#endif
#include <stdio.h>
#include <string.h>

#include "command.h"
#include "controller.h"
#include "event_queue.h"
#include "log.h"
#include "player.h"
#include "window.h"
#include "world.h"

#ifdef GAME_MAX_FRAMERATE
#include <unistd.h>
#endif

typedef struct {
    ControllerArray *controllers;
    Player players[4];
    World *world;
    uint8_t controllers_count;
    uint8_t number_players;
    bool running;
    bool show_debug_info;
    bool command_mode;
} Game;

static Game game;

void game_init(const uint8_t number_players, const uint32_t world_seed,
               const bool force_tty, const bool force_no_tty) {
    assert(0 < number_players && number_players <= 4);

    game.running = true;
    game.show_debug_info = GAME_DEFAULT_SHOW_DEBUG_INFO;
    game.command_mode = false;

    game.world = world_create(world_seed);

    window_init(force_tty, force_no_tty);

    event_queue_init();

    game.controllers = controller_get_connected_controllers();
    controller_start_monitor();

    game.number_players = number_players;
    for (uint8_t i = 0; i < number_players; ++i) {
        Controller *const controller =
            i < game.controllers->length ? game.controllers->array[i] : NULL;
        player_init(&game.players[i], i, game.number_players, controller,
                    game.world);
    }
}

void game_quit(void) {
    for (uint8_t i = 0; i < game.number_players; ++i) {
        player_destroy(&game.players[i]);
    }
    controller_stop_monitor();
    controller_array_destroy(game.controllers);
    event_queue_quit();
    world_destroy(game.world);
    window_quit();
    log_debugf("quit");
}

[[gnu::nonnull]]
static inline void game_handle_button_down_event(
    const ButtonEvent *const button_down_event) {
    assert(button_down_event != NULL);

    switch (button_down_event->button) {
        case CONTROLLER_BUTTON_ZL: {
            const Player *const player =
                &game.players[button_down_event->player_index];
            player_place_block(player, game.players, game.number_players,
                               game.world);
            break;
        }

        case CONTROLLER_BUTTON_ZR: {
            const Player *const player =
                &game.players[button_down_event->player_index];
            player_break_block(player, game.world);
            break;
        }

        case CONTROLLER_BUTTON_R:
            game.world->place_block++;
            if (game.world->place_block == BLOCK_TYPE_COUNT) {
                game.world->place_block = 1;
            }
            break;

        case CONTROLLER_BUTTON_L:
            game.world->place_block--;
            if (game.world->place_block == 0) {
                game.world->place_block = BLOCK_TYPE_COUNT - 1;
            }
            break;

        case CONTROLLER_BUTTON_HOME:
            game.running = false;
            break;

        case CONTROLLER_BUTTON_PLUS:
            game.show_debug_info = !game.show_debug_info;
            break;

        default:
            break;
    }
}

[[gnu::nonnull]]
static inline void game_handle_button_up_event(
    [[maybe_unused]] const ButtonEvent *const button_up_event) {
    assert(button_up_event != NULL);
}

static inline void game_add_player_command(void) {
    if (game.number_players == 4) return;  // TODO: display error message

    Controller *controller = NULL;
    for (size_t i = 0; i < game.controllers->length; ++i) {
        if (controller_get_player_index(game.controllers->array[i]) == -1) {
            controller = game.controllers->array[i];
            break;
        }
    }

    const uint8_t new_number_players = game.number_players + 1;
    player_init(&game.players[game.number_players], game.number_players,
                new_number_players, controller, game.world);
    for (uint8_t i = 0; i < game.number_players; ++i) {
        player_update_viewport(&game.players[i], new_number_players);
    }
    game.number_players = new_number_players;
}

[[gnu::nonnull]]
static inline void game_execute_command(const Command *const command) {
    assert(command != NULL);

    switch (command->command_name) {
        case COMMAND_ADD_PLAYER:
            game_add_player_command();
            break;

        case COMMAND_GAME_MODE:
            player_set_game_mode(
                &game.players[command->game_mode_command.player_index],
                command->game_mode_command.game_mode);
            break;

        case COMMAND_QUIT:
            game.running = false;
            break;

        case COMMAND_TP:
            // TODO: handle teleporting other players
            log_debugf("executing command: tp player=%u x=%d y=%d z=%d",
                       0,  // command->tp_command.player_index,
                       command->tp_command.position.x,
                       command->tp_command.position.y,
                       command->tp_command.position.z);
            player_teleport(&game.players[0], command->tp_command.position,
                            game.world);
            break;

        default:
            assert(false && "unreachable");
    }
}

static void game_enter_command_mode(void) {
    assert(window.is_init);
    game.command_mode = true;
    window.show_cursor = true;
    window.cursor_position.x = 1;
    window.cursor_position.y = window.height - 1;
}

static void game_quit_command_mode(void) {
    assert(window.is_init);
    game.command_mode = false;
    window.show_cursor = false;
    command_clear();
}

[[gnu::nonnull]]
static inline void game_handle_char_event(const CharEvent *const char_event) {
    assert(char_event != NULL);
#ifndef __wasm__
    if (char_event->chr == CHAR_EVENT_CTRL_Z) {
        log_debugf("suspending");
        raise(SIGTSTP);
        return;
    }
#endif

    if (game.command_mode) {
        if (char_event->chr == CHAR_EVENT_KEY_ESCAPE ||
            char_event->chr == CHAR_EVENT_CTRL_C) {
            game_quit_command_mode();
            return;
        }

        if (char_event->chr == CHAR_EVENT_KEY_ENTER) {
            Command command;
            if (command_parse(&command)) {
                game_execute_command(&command);
            } else {
                // TODO: displayer error message
            }
            game_quit_command_mode();
            return;
        }

        if (char_event->chr == CHAR_EVENT_KEY_BACKSPACE) {
            command_erase_char();
            --window.cursor_position.x;
            return;
        }

        if (command_is_valid_char(char_event->chr)) {
            command_append(char_event->chr);
            ++window.cursor_position.x;
            return;
        }

        return;
    }

    switch (char_event->chr) {
        case CHAR_EVENT_KEY_COLON:
            game_enter_command_mode();
            break;

#ifdef GAME_KEYBOARD_CONTROL
        case CHAR_EVENT_KEY_A:
            player_break_block(&game.players[0], game.world);
            break;

        case CHAR_EVENT_KEY_E:
            player_place_block(&game.players[0], game.players,
                               game.number_players, game.world);
            break;

        case CHAR_EVENT_KEY_R:
            game.world->place_block++;
            if (game.world->place_block == BLOCK_TYPE_COUNT) {
                game.world->place_block = 1;
            }
            break;

        case CONTROLLER_BUTTON_L:
            game.world->place_block--;
            if (game.world->place_block == 0) {
                game.world->place_block = BLOCK_TYPE_COUNT - 1;
            }
            break;

        case CHAR_EVENT_KEY_SPACE:
            if (game.players[0].game_mode != PLAYER_GAME_MODE_SURVIVAL) {
                game.players[0].input_velocity.y +=
                    PLAYER_KEYBOARD_MOVEMENT_SPEED;
            } else {
                player_jump(&game.players[0]);
            }
            break;

        case CHAR_EVENT_KEY_B:
            if (game.players[0].game_mode != PLAYER_GAME_MODE_SURVIVAL) {
                game.players[0].input_velocity.y -=
                    PLAYER_KEYBOARD_MOVEMENT_SPEED;
            }
            break;

        case CHAR_EVENT_KEY_K:
            game.players[0].input_velocity.z += PLAYER_KEYBOARD_MOVEMENT_SPEED;
            break;

        case CHAR_EVENT_KEY_J:
            game.players[0].input_velocity.z -= PLAYER_KEYBOARD_MOVEMENT_SPEED;
            break;

        case CHAR_EVENT_KEY_H:
            game.players[0].input_velocity.x -= PLAYER_KEYBOARD_MOVEMENT_SPEED;
            break;

        case CHAR_EVENT_KEY_L:
            game.players[0].input_velocity.x += PLAYER_KEYBOARD_MOVEMENT_SPEED;
            break;

        case CHAR_EVENT_KEY_Z:
            player_rotate(&game.players[0], (v2f){
                                                0.0f,
                                                CAMERA_KEYBOARD_SENSITIVITY,
                                            });
            break;

        case CHAR_EVENT_KEY_S:
            player_rotate(&game.players[0], (v2f){
                                                0.0f,
                                                -CAMERA_KEYBOARD_SENSITIVITY,
                                            });
            break;

        case CHAR_EVENT_KEY_Q:
            player_rotate(&game.players[0], (v2f){
                                                CAMERA_KEYBOARD_SENSITIVITY,
                                                0.0f,
                                            });
            break;

        case CHAR_EVENT_KEY_D:
            player_rotate(&game.players[0], (v2f){
                                                -CAMERA_KEYBOARD_SENSITIVITY,
                                                0.0f,
                                            });
            break;
#endif
    }
}

static inline void game_handle_resize_event(void) {
    assert(window.is_init);
    for (uint8_t i = 0; i < game.number_players; ++i) {
        player_update_viewport(&game.players[i], game.number_players);
    }
}

[[gnu::nonnull]]
static inline void game_hanlde_controller_connect_event(
    Controller *const controller) {
    assert(controller != NULL);

    controller_array_push(game.controllers, controller);
    for (int8_t i = 0; i < 4; ++i) {
        Player *const player = &game.players[i];
        if (player->controller == NULL) {
            player->controller = controller;
            controller_set_player_index(controller, i);
            log_debugf("assign controller '%s' to player %d",
                       controller_get_name(controller), i);
            break;
        }
    }
}

static inline void game_update(const float delta_time_seconds) {
    window_update();

    while (!event_queue_is_empty()) {
        const Event *const event = event_queue_get();
        switch (event->type) {
            case EVENT_TYPE_BUTTON_DOWN:
                game_handle_button_down_event(&event->button_event);
                break;

            case EVENT_TYPE_BUTTON_UP:
                game_handle_button_up_event(&event->button_event);
                break;

            case EVENT_TYPE_CHAR:
                game_handle_char_event(&event->char_event);
                break;

            case EVENT_TYPE_CONTROLLER_CONNECT:
                game_hanlde_controller_connect_event(
                    event->controller_event.controller);
                break;

            case EVENT_TYPE_CONTROLLER_DISCONNECT: {
                Controller *const controller =
                    event->controller_event.controller;
                const int8_t player_index =
                    controller_get_player_index(controller);
                if (player_index != -1) {
                    game.players[player_index].controller = NULL;
                }
                for (size_t i = 0; i < game.controllers->length; ++i) {
                    if (game.controllers->array[i] == controller) {
                        controller_array_remove(game.controllers, i);
                        break;
                    }
                }
                controller_destroy(controller);
                break;
            }

            case EVENT_TYPE_RESIZE:
                game_handle_resize_event();
                break;

            default:
                assert(false && "unreachable");
        }
        event_queue_next();
    }

    for (uint8_t i = 0; i < game.number_players; ++i) {
        Player *const player = &game.players[i];
        const Controller *const controller = player->controller;

        if (controller == NULL) continue;

        const v2f left_stick_value =
            controller_get_stick(controller, CONTROLLER_STICK_LEFT);
        player->input_velocity.x +=
            left_stick_value.x * PLAYER_MOVEMENT_SPEED * delta_time_seconds;
        player->input_velocity.z += left_stick_value.y * -1.0f *
                                    PLAYER_MOVEMENT_SPEED * delta_time_seconds;

        const v2f right_stick_value =
            v2f_mul(controller_get_stick(controller, CONTROLLER_STICK_RIGHT),
                    -1.0f * CAMERA_SENSITIVITY * delta_time_seconds);
        player_rotate(player, right_stick_value);

        if (player->game_mode == PLAYER_GAME_MODE_SURVIVAL) {
            if (controller_get_button(controller, CONTROLLER_BUTTON_A)) {
                player_jump(player);
            }
        } else {
            player->input_velocity.y +=
                PLAYER_MOVEMENT_SPEED * delta_time_seconds *
                (controller_get_button(controller, CONTROLLER_BUTTON_A) -
                 controller_get_button(controller, CONTROLLER_BUTTON_B));
        }
    }

    for (uint8_t i = 0; i < game.number_players; ++i) {
        player_update(&game.players[i], game.world, delta_time_seconds);
    }
}

static inline void game_render_debug_info(const float delta_time_seconds) {
    assert(delta_time_seconds >= 0.0f);
    assert(window.is_init);

    char buffer[19];
    v2i position = {.x = window.width - sizeof(buffer) - 1, .y = 1};
    window_render_string(position, "+----------------+", COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
    ++position.y;
#ifdef PROD
    window_render_string(position, "| prod build     |", COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
#else
    window_render_string(position, "| debug build    |", COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
#endif
    ++position.y;
    snprintf(buffer, sizeof(buffer), "| fps: %9.2f |",
             1.0f / delta_time_seconds);
    window_render_string(position, buffer, COLOR_WHITE, WINDOW_Z_BUFFER_FRONT);
    ++position.y;
    snprintf(buffer, sizeof(buffer), "| dt: %7.2f ms |",
             delta_time_seconds * 1000.0f);
    window_render_string(position, buffer, COLOR_WHITE, WINDOW_Z_BUFFER_FRONT);
    ++position.y;
    window_render_string(position, "+----------------+", COLOR_WHITE,
                         WINDOW_Z_BUFFER_FRONT);
}

static inline void game_render_player_screens_borders(void) {
    assert(window.is_init);

    if (game.number_players == 1) return;

    const bool window_is_landscape =
        CHARACTER_RATIO * window.width / window.height >= 1.0f;

    const int half_width = window.width / 2;
    const int half_height = window.height / 2;
    const int half_height_index = half_height * window.width;

    if (window_is_landscape || game.number_players == 4) {
        for (int y = 0; y < window.height; ++y) {
            window_set_pixel(y * window.width + half_width, '|',
                             GAME_PLAYER_SCREENS_BORDER_COLOR,
                             WINDOW_Z_BUFFER_FRONT);
        }
    }

    if (!window_is_landscape || game.number_players == 4) {
        for (int x = 0; x < window.width; ++x) {
            window_set_pixel(half_height_index + x, '-',
                             GAME_PLAYER_SCREENS_BORDER_COLOR,
                             WINDOW_Z_BUFFER_FRONT);
        }
    }

    if (game.number_players > 2) {
        if (game.number_players == 3) {
            if (window_is_landscape) {
                for (int x = half_width + 1; x < window.width; ++x) {
                    window_set_pixel(half_height_index + x, '-',
                                     GAME_PLAYER_SCREENS_BORDER_COLOR,
                                     WINDOW_Z_BUFFER_FRONT);
                }
            } else {
                for (int y = half_height + 1; y < window.height; ++y) {
                    window_set_pixel(y * window.width + half_width, '|',
                                     GAME_PLAYER_SCREENS_BORDER_COLOR,
                                     WINDOW_Z_BUFFER_FRONT);
                }
            }
        }

        window.pixels[half_height_index + half_width].chr = '+';
    }
}

static inline void game_render_ui(const float delta_time_seconds) {
    assert(delta_time_seconds >= 0.0f);

    if (game.show_debug_info) game_render_debug_info(delta_time_seconds);

    game_render_player_screens_borders();

    for (uint8_t i = 0; i < game.number_players; ++i) {
        player_render_ui(&game.players[i], game.number_players, game.world);
    }

    if (game.command_mode) command_render();
}

[[gnu::nonnull]]
static void *game_player_render_thread(void *const data) {
    assert(data != NULL);
    const Player *const player = data;
    const Camera *const camera = player->camera;
    const Viewport *const viewport = &player->viewport;

    for (uint8_t j = 0; j < game.number_players; ++j) {
        if (player->player_index == j) continue;
        player_render(&game.players[j], camera, viewport);
    }
    world_render(game.world, camera, viewport);
    return NULL;
}

#ifndef __wasm__
static inline void game_render_multiplayer(void) {
    pthread_t render_threads[game.number_players - 1];
    for (int8_t i = 1; i < game.number_players; ++i) {
        const int return_code =
            pthread_create(&render_threads[i - 1], NULL,
                           game_player_render_thread, &game.players[i]);
        if (return_code < 0) {
            log_errorf("failed to create render thread: %s",
                       strerror(-return_code));
            exit(EXIT_FAILURE);
        }
    }

    game_player_render_thread(&game.players[0]);

    for (int8_t i = 1; i < game.number_players; ++i) {
        const int return_code = pthread_join(render_threads[i - 1], NULL);
        if (return_code < 0) {
            log_errorf("failed to join render thread: %s",
                       strerror(-return_code));
            exit(EXIT_FAILURE);
        }
    }
}
#else
static inline void game_render_multiplayer(void) {
    for (int8_t i = 1; i < game.number_players; ++i) {
        game_player_render_thread(&game.players[i]);
    }
}
#endif

static inline void game_render(const float delta_time_seconds) {
    assert(delta_time_seconds >= 0.0f);

    window_clear();

    game_render_ui(delta_time_seconds);

    if (game.number_players > 1) {
        game_render_multiplayer();
    } else {
        game_player_render_thread(&game.players[0]);
    }

    window_flush();
}

void game_run(void) {
    assert(game.running);

    uint64_t frame_start_time_microseconds = get_time_microseconds();
    float delta_time_seconds = 0.0f;

    log_debugf("game launched");
    while (game.running) {
        game_update(delta_time_seconds);
        game_render(delta_time_seconds);

#ifdef GAME_MAX_FRAMERATE
        const uint64_t delta_time_microseconds =
            get_time_microseconds() - frame_start_time_microseconds;
        if (delta_time_microseconds < 1000000 / GAME_MAX_FRAMERATE) {
            usleep((1000000 / GAME_MAX_FRAMERATE) - delta_time_microseconds);
        }
#endif
        const uint64_t frame_end_time_microseconds = get_time_microseconds();
        delta_time_seconds =
            (frame_end_time_microseconds - frame_start_time_microseconds) /
            1000000.0f;
        frame_start_time_microseconds = frame_end_time_microseconds;
    }
}
