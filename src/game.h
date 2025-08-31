#pragma once

#include <stdbool.h>
#include <stdint.h>

void game_init(const uint8_t number_players, const uint32_t world_seed,
               const bool force_tty, const bool force_no_tty);
void game_quit(void);
void game_run(void);
