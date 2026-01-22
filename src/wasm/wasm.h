#pragma once

#include <stdint.h>

[[gnu::nonnull(2)]]
int main(const int argc, char *argv[]);

[[gnu::nonnull]]
void run_callback(void (*callback)(void));

[[gnu::nonnull]]
void run_callback_uint32(void (*callback)(const uint32_t),
                         const uint32_t param);

void wasm_main(void);

void malloc_init(void);
