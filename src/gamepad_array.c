#include "gamepad_array.h"

#include "gamepad.h"

ARRAY_IMPLEMENTATION(gamepad, Gamepad, Gamepad *)

void gamepad_array_destroy(GamepadArray *const array) {
    assert(array != NULL);
    for (size_t i = 0; i < array->length; ++i) {
        Gamepad *const gamepad = array->array[i];
        assert(gamepad != NULL);
        gamepad_destroy(gamepad);
    }
    array_destroy((Array *)array);
}
