#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "player_game_mode.h"
#include "vec.h"

typedef enum : uint8_t {
    COMMAND_ADD_PLAYER,
    COMMAND_GAME_MODE,
    COMMAND_QUIT,
    // COMMAND_REMOVE_PLAYER,
    // COMMAND_SET_CONTROLLER,
    COMMAND_TP,
} CommandName;

typedef struct {
    uint8_t player_index;
    PlayerGameMode game_mode;
} CommandGameMode;

typedef struct {
    v3i position;
    uint8_t player_index;
} CommandTp;

typedef struct {
    union {
        CommandGameMode game_mode_command;
        CommandTp tp_command;
    };
    CommandName command_name;
} Command;

char *command_get_error_message(void);

void command_append(const char chr);

void command_erase_char(void);

[[gnu::nonnull]]
bool command_parse(Command *const command);

bool command_is_valid_char(const char chr);

void command_clear(void);

void command_render(void);
