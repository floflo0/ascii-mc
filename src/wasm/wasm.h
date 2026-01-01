#pragma once

[[gnu::nonnull(2)]]
int main(const int argc, char *argv[]);

[[gnu::nonnull]]
void run_callback(void (*callback)(void));

[[gnu::nonnull(1)]]
void run_callback_int(void (*callback)(const int), const int param);

[[gnu::nonnull(1)]]
void run_callback_ptr(void (*callback)(void *const), void *const param);

void wasm_main(void);

void malloc_init(void);
