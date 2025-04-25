#pragma once

#include <stdint.h>

void xorshift32_set_seed(const uint32_t seed);
uint32_t xorshift32(void);
