#pragma once
#ifdef __wasm__

#include <stdbool.h>

#define KEYBOARD_JUMP_KEY ' '
#define KEYBOARD_CROUCH_KEY 16  // Shift
#define KEYBOARD_UP_KEY 'Z'
#define KEYBOARD_DOWN_KEY 'S'
#define KEYBOARD_LEFT_KEY 'Q'
#define KEYBOARD_RIGHT_KEY 'D'
#define KEYBOARD_BLOCK_PREVIOUS_KEY 'A'
#define KEYBOARD_BLOCK_NEXT_KEY 'E'

void mouse_and_keyboard_init(void);
void mouse_and_keyboard_quit(void);
bool keyboard_is_key_press(const char key);

#else
#define mouse_and_keyboard_init()
#define mouse_and_keyboard_quit()
#endif
