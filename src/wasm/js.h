#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

[[gnu::nonnull]]
extern void JS_console_log(const char *string);
[[gnu::nonnull]]
extern void JS_console_error(const char *string);

[[gnu::nonnull]]
extern void JS_requestAnimationFrame(void (*callback)(void));

extern uint64_t JS_Date_now(void);
extern double JS_performance_now(void);

[[gnu::nonnull(1)]]
extern void JS_write(const void *const buf, const size_t count);
extern char JS_read_char(void);

extern size_t JS_get_memory_size(void);

extern int JS_get_terminal_width(void);
extern int JS_get_terminal_height(void);

extern void JS_ongamepadconnected(void (*callback)(const uint32_t));
extern void JS_ongamepaddisconnected(void (*callback)(const uint32_t));
extern void JS_gameapad_rumble(const uint32_t gamepad_index,
                               const uint16_t low_frequency,
                               const uint16_t high_frequency,
                               const uint32_t duration_miliseconds);
extern size_t JS_get_gamepad_buttons_count(const uint32_t gamepad_index);
extern size_t JS_get_gamepad_axis_count(const uint32_t gamepad_index);
extern void JS_get_gamepad_id(const uint32_t gamepad_index,
                              char *const gamepad_id,
                              const size_t gamepad_id_size);
extern float JS_Gamepad_get_axis(const uint32_t gamepad_index,
                                 const uint8_t axis_index);
extern bool JS_get_gamepad_button(const uint32_t gamepad_index,
                                  const uint8_t button);
