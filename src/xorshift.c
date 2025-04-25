#include "xorshift.h"

static uint32_t xorshift32_state = 0;

void xorshift32_set_seed(const uint32_t seed) {
    xorshift32_state = seed;
}

uint32_t xorshift32(void) {
    uint32_t x = xorshift32_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return xorshift32_state = x;
}
