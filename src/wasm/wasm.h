#pragma once

#include <stdint.h>

[[gnu::nonnull(2)]]
int main(const int argc, char *argv[]);

[[gnu::nonnull]]
void run_callback(void (*callback)(void));

[[gnu::nonnull]]
void run_callback_uint32(void (*callback)(const uint32_t),
                         const uint32_t param);

[[gnu::nonnull]]
void run_callback_int(void (*callback)(const int), const int param);

[[gnu::nonnull]]
void run_callback_int_int(void (*callback)(const int, const int),
                          const int param1, const int param2);

void wasm_main(void);

void malloc_init(void);
