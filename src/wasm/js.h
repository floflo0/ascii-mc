#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define RETURNS_NONNULL __attribute__((returns_nonnull))

#define JS_NULL -1

typedef int JS_Object;
typedef JS_Object JS_Array;
typedef JS_Array JS_GamepadArray;
typedef JS_Object JS_Gamepad;

extern void JS_Array_foreach(const JS_Array array,
                             void (*callback)(const JS_Object)) NONNULL(2);

extern void JS_console_log(const char *string) NONNULL();
extern void JS_console_error(const char *string) NONNULL();

extern int JS_requestAnimationFrame(void (*callback)(void)) NONNULL();
extern int JS_requestAnimationFrame_data(void (*callback)(void *const),
                                         void *const data) NONNULL(1);
extern void JS_cancelAnimationFrame(const int requestId);

extern uint64_t JS_Date_now(void);
extern double JS_performance_now(void);

extern double JS_Math_pow(const double x, const double y);
extern double JS_Math_cos(const double x);
extern double JS_Math_sin(const double x);
extern double JS_Math_tan(const double x);
extern double JS_Math_atan(const double x);
extern double JS_Math_round(const double x);
extern bool JS_isNaN(const double x);
extern double JS_fmod(const double x, const double y);

extern JS_GamepadArray JS_navigator_getGamepads(void);
extern JS_Gamepad JS_GamepadArray_get(const JS_GamepadArray array,
                                      const int gamepad_index);
extern void JS_Gamepad_get_id(const JS_Gamepad gamepad, char *id,
                              const size_t id_size) NONNULL(2);
extern int JS_Gamepad_get_index(const JS_Gamepad gamepad);
extern bool JS_Gamepad_get_connected(const JS_Gamepad gamepad);
extern float JS_Gamepad_get_axe(const JS_Gamepad gamepad, const int axe_index);
extern bool JS_Gamepad_get_button(const JS_Gamepad gamepad,
                                  const int button_index);
extern size_t JS_Gamepad_get_axes_length(const JS_Gamepad gamepad);

extern void JS_Object_free(const JS_Object object);

extern void JS_write(const void *const buf, const size_t count) NONNULL(1);
extern void JS_exit(const int status) __attribute__((__noreturn__));
extern char JS_read_char(void);
extern void JS_usleep(uint32_t usec);

extern void *JS_get_heap_base(void) RETURNS_NONNULL;
extern size_t JS_get_memory_size(void);

extern int JS_get_terminal_width(void);
extern int JS_get_terminal_height(void);

extern void JS_wait_for_next_frame(void);
