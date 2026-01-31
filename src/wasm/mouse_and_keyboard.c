// Help the lsp
#ifndef __wasm__
#define __wasm__
#endif

#include "mouse_and_keyboard.h"

#include <assert.h>

#include "../event_queue.h"
#include "js.h"

static bool key_states[256] = {0};

static void handle_js_pointer_move_event(const int movement_x,
                                         const int movement_y) {
    event_queue_push(&(Event){
        .type = EVENT_TYPE_MOUSE_MOVE,
        .mouse_move_event =
            {
                .movement_x = movement_x,
                .movement_y = movement_y,
            },
    });
}

static void handle_js_pointer_down_event(const int button) {
    event_queue_push(&(Event){
        .type = EVENT_TYPE_MOUSE_BUTTON_DOWN,
        .mouse_button_event = {.button = (MouseButton)button},
    });
}

static void handle_js_keydown_event(const int key) {
    assert(0 <= key && key < 128);
    key_states[key] = true;
    event_queue_push(&(Event){
        .type = EVENT_TYPE_KEY_DOWN,
        .keyboard_event = {.key = key},
    });
}

static void handle_js_keyup_event(const int key) {
    assert(0 <= key && key < 128);
    key_states[key] = false;
    event_queue_push(&(Event){
        .type = EVENT_TYPE_KEY_UP,
        .keyboard_event = {.key = key},
    });
}

void mouse_and_keyboard_init(void) {
    JS_on_pointer_move(handle_js_pointer_move_event);
    JS_on_pointer_down(handle_js_pointer_down_event);
    JS_on_keydown(handle_js_keydown_event);
    JS_on_keyup(handle_js_keyup_event);
}

void mouse_and_keyboard_quit(void) {
    JS_on_pointer_move(NULL);
    JS_on_pointer_down(NULL);
    JS_on_keydown(NULL);
    JS_on_keyup(NULL);
}

bool keyboard_is_key_press(const char key) {
    return key_states[(size_t)key];
}
