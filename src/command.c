#include "command.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "log.h"
#include "utils.h"
#include "window.h"

#define COMMAND_CAPACITY 1024
#define ERROR_MESSAGE_CAPACITY 2048
static_assert(COMMAND_CAPACITY < ERROR_MESSAGE_CAPACITY,
              "An error message must be able to contain the command");

#define WORLD_ORIGIN (WORLD_SIZE / 2)
#define WORLD_MIN_X (-WORLD_ORIGIN * CHUNK_SIZE)
#define WORLD_MAX_X ((WORLD_SIZE - WORLD_ORIGIN) * CHUNK_SIZE - 1)
#define WORLD_MIN_Y 0
#define WORLD_MAX_Y CHUNK_HEIGHT
#define WORLD_MIN_Z WORLD_MIN_X
#define WORLD_MAX_Z WORLD_MAX_X

static char command_buffer[COMMAND_CAPACITY] = {0};
static size_t command_buffer_length = 0;
static bool error_message = false;
static char error_message_string[ERROR_MESSAGE_CAPACITY];

[[gnu::nonnull]]
static inline void set_error_message(const char *const new_error_message) {
    assert(new_error_message != NULL);

    error_message = true;
    copy_string(error_message_string, new_error_message,
                ERROR_MESSAGE_CAPACITY);
}

[[gnu::nonnull(1)]] [[gnu::format(printf, 1, 2)]]
static inline void set_error_message_format(
    const char *const error_message_format, ...) {
    assert(error_message_format != NULL);
    error_message = true;
    va_list args;
    va_start(args, error_message_format);
    vsnprintf(error_message_string, ERROR_MESSAGE_CAPACITY,
              error_message_format, args);
    va_end(args);
}

char *command_get_error_message(void) {
    if (error_message) return error_message_string;

    return NULL;
}

void command_append(const char chr) {
    assert(command_is_valid_char(chr));

    if (command_buffer_length == COMMAND_CAPACITY - 1) return;

    command_buffer[command_buffer_length++] = chr;
    command_buffer[command_buffer_length] = '\0';
}

void command_erase_char(void) {
    if (command_buffer_length == 0) return;

    --command_buffer_length;
    command_buffer[command_buffer_length] = '\0';
}

[[gnu::nonnull]]
static bool parse_game_mode_command(Command *const command) {
    const char *const game_mode_argument = strtok(NULL, " ");
    if (game_mode_argument == NULL) {
        set_error_message("game_mode: missing argument: game mode");
        return false;
    }

    PlayerGameMode game_mode;
    if (strcmp(game_mode_argument, "survival") == 0) {
        game_mode = PLAYER_GAME_MODE_SURVIVAL;
    } else if (strcmp(game_mode_argument, "creative") == 0) {
        game_mode = PLAYER_GAME_MODE_CREATIVE;
    } else if (strcmp(game_mode_argument, "spectator") == 0) {
        game_mode = PLAYER_GAME_MODE_SPECTATOR;
    } else {
        set_error_message_format("game_mode: invalid argument: '%s'",
                                 game_mode_argument);
        return false;
    }

    if (strtok(NULL, " ") != NULL) {
        set_error_message("game_mode: too many arguments");
        return false;
    }

    command->command_name = COMMAND_GAME_MODE;
    command->game_mode_command.player_index = 0;
    command->game_mode_command.game_mode = game_mode;
    return true;
}

[[gnu::nonnull]]
static bool parse_tp_command(Command *const command) {
    assert(command != NULL);
    // int player_index;
    int x, y, z;
    const char *const x_string = strtok(NULL, " ");
    // const char *argument2 = strtok(NULL, " ");
    // const char *argument3 = strtok(NULL, " ");
    // const char *argument4 = strtok(NULL, " ");

    // if (argument4 != NULL) {
    //     assert(argument1 != NULL);
    //
    //     if (!parse_int(argument1, &player_index)) {
    //         set_error_message_format(
    //             "tp: argument player_index: '%s' is not a valid integer",
    //             argument1
    //         );
    //         return false;
    //     }
    //
    //     if (
    // }
    if (x_string == NULL) {
        set_error_message("tp: missing argument: x");
        return false;
    }

    if (!parse_int(x_string, &x)) {
        set_error_message_format("tp: argument x: '%s' is not a valid integer",
                                 x_string);
        return false;
    }

    if (WORLD_MIN_X > x || x > WORLD_MAX_X) {
        set_error_message_format("tp: argument x must be between %d and %d",
                                 WORLD_MIN_X, WORLD_MAX_X);
        return false;
    }

    const char *const y_string = strtok(NULL, " ");
    if (y_string == NULL) {
        set_error_message("tp: missing argument: y");
        return false;
    }

    if (!parse_int(y_string, &y)) {
        set_error_message_format("tp: argument y: '%s' is not a valid integer",
                                 y_string);
        return false;
    }

    const char *const z_string = strtok(NULL, " ");
    if (z_string == NULL) {
        z = y;
        y = -1;
    } else {
        if (WORLD_MIN_Y > y || y > WORLD_MAX_Y) {
            set_error_message_format("tp: argument y must be between %d and %d",
                                     WORLD_MIN_Y, WORLD_MAX_Y);
            return false;
        }

        if (!parse_int(z_string, &z)) {
            set_error_message_format(
                "tp: argument z: '%s' is not a valid integer", z_string);
            return false;
        }

        if (strtok(NULL, " ") != NULL) {
            set_error_message("tp: too many arguments");
            return false;
        }
    }

    if (WORLD_MIN_Z > z || z > WORLD_MAX_Z) {
        set_error_message_format("tp: argument z must be between %d and %d",
                                 WORLD_MIN_Z, WORLD_MAX_Z);
        return false;
    }

    command->command_name = COMMAND_TP;
    command->tp_command.position.x = x;
    command->tp_command.position.y = y;
    command->tp_command.position.z = z;
    return true;
}

bool command_parse(Command *const command) {
    assert(command != NULL);

    error_message = false;

    log_debugf("parsing command '%s'", command_buffer);

    const char *token = strtok(command_buffer, " ");

    if (token == NULL) return false;

    if (strcmp(token, "add_player") == 0) {
        if (strtok(NULL, " ") != NULL) {
            set_error_message("add_player: too many arguments");
            return false;
        }
        command->command_name = COMMAND_ADD_PLAYER;
        return true;
    }

    if (strcmp(token, "game_mode") == 0)
        return parse_game_mode_command(command);

    if (strcmp(token, "q") == 0 || strcmp(token, "quit") == 0) {
        if (strtok(NULL, " ") != NULL) {
            set_error_message("quit: too many arguments");
            return false;
        }
        command->command_name = COMMAND_QUIT;
        return true;
    }

    if (strcmp(token, "tp") == 0) return parse_tp_command(command);

    set_error_message_format("unknow command: '%s'", token);
    return false;
}

inline bool command_is_valid_char(const char chr) {
    return isalnum(chr) || ispunct(chr) || chr == ' ';
}

void command_clear(void) {
    error_message = false;
    command_buffer_length = 0;
    command_buffer[0] = '\0';
}

void command_render(void) {
    v2i position = {
        .x = 0,
        .y = window.height - 1,
    };
    window_render_rectangle(position, (v2i){window.width, 1}, ' ', COLOR_WHITE,
                            WINDOW_Z_BUFFER_FRONT);

    window_set_pixel(position.y * window.width, ':', GAME_COMMAND_COLOR,
                     WINDOW_Z_BUFFER_FRONT);

    ++position.x;
    window_render_string(position, command_buffer, GAME_COMMAND_COLOR,
                         WINDOW_Z_BUFFER_FRONT);
}
