#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define JS_NULL -1

typedef int JS_Object;
typedef JS_Object JS_Array;
typedef JS_Array JS_GamepadArray;
typedef JS_Object JS_Gamepad;

[[gnu::nonnull(2)]]
extern void JS_Array_foreach(const JS_Array array,
                             void (*callback)(const JS_Object));

[[gnu::nonnull]]
extern void JS_console_log(const char *string);
[[gnu::nonnull]]
extern void JS_console_error(const char *string);

[[gnu::nonnull]]
extern int JS_requestAnimationFrame(void (*callback)(void));
[[gnu::nonnull(1)]]
extern int JS_requestAnimationFrame_data(void (*callback)(void *const),
                                         void *const data);
extern void JS_cancelAnimationFrame(const int requestId);

extern uint64_t JS_Date_now(void);
extern double JS_performance_now(void);

extern JS_GamepadArray JS_navigator_getGamepads(void);
extern JS_Gamepad JS_GamepadArray_get(const JS_GamepadArray array,
                                      const int gamepad_index);
[[gnu::nonnull(2)]]
extern void JS_Gamepad_get_id(const JS_Gamepad gamepad, char *id,
                              const size_t id_size);
extern int JS_Gamepad_get_index(const JS_Gamepad gamepad);
extern bool JS_Gamepad_get_connected(const JS_Gamepad gamepad);
[[gnu::const]]
extern float JS_Gamepad_get_axe(const JS_Gamepad gamepad, const int axe_index);
extern bool JS_Gamepad_get_button(const JS_Gamepad gamepad,
                                  const int button_index);
extern size_t JS_Gamepad_get_axes_length(const JS_Gamepad gamepad);

extern void JS_Object_free(const JS_Object object);

[[gnu::nonnull(1)]]
extern void JS_write(const void *const buf, const size_t count);
extern char JS_read_char(void);

extern size_t JS_get_memory_size(void);

extern int JS_get_terminal_width(void);
extern int JS_get_terminal_height(void);
