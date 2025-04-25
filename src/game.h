#pragma once

#include <stdint.h>

void game_init(const uint8_t number_players, const uint32_t world_seed);
void game_quit(void);
void game_run(void);
