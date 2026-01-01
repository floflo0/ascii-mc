#pragma once

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

int main(const int argc, char *argv[]) NONNULL(2);

void run_callback(void (*callback)(void)) NONNULL();

void run_callback_int(void (*callback)(const int), const int param) NONNULL(1);

void run_callback_ptr(void (*callback)(void *const), void *const param)
    NONNULL(1);

void wasm_main(void);

void malloc_init(void);
