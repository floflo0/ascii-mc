#pragma once

#include "controller_defs.h"

#define CHAR_EVENT_CTRL_C 0x03
#define CHAR_EVENT_CTRL_Z 0x1a

#define CHAR_EVENT_KEY_ESCAPE 27
#define CHAR_EVENT_KEY_ENTER '\r'
#define CHAR_EVENT_KEY_BACKSPACE 127

#define CHAR_EVENT_KEY_SPACE ' '
#define CHAR_EVENT_KEY_A 'a'
#define CHAR_EVENT_KEY_B 'b'
#define CHAR_EVENT_KEY_D 'd'
#define CHAR_EVENT_KEY_E 'e'
#define CHAR_EVENT_KEY_H 'h'
#define CHAR_EVENT_KEY_J 'j'
#define CHAR_EVENT_KEY_K 'k'
#define CHAR_EVENT_KEY_L 'l'
#define CHAR_EVENT_KEY_Q 'q'
#define CHAR_EVENT_KEY_R 'r'
#define CHAR_EVENT_KEY_S 's'
#define CHAR_EVENT_KEY_Z 'z'
#define CHAR_EVENT_KEY_COLON ':'

typedef enum : uint8_t {
    EVENT_TYPE_BUTTON_DOWN,
    EVENT_TYPE_BUTTON_UP,
    EVENT_TYPE_CHAR,
    EVENT_TYPE_CONTROLLER_CONNECT,
    EVENT_TYPE_CONTROLLER_DISCONNECT,
    EVENT_TYPE_RESIZE,
} EventType;

typedef struct {
    uint8_t player_index;
    ControllerButton button;
} ButtonEvent;

typedef struct {
    char chr;
} CharEvent;

typedef struct {
    Controller *controller;
} ControllerEvent;

typedef struct {
    union {
        ButtonEvent button_event;
        CharEvent char_event;
        ControllerEvent controller_event;
    };
    EventType type;
} Event;
