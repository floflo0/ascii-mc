#pragma once

#include "array.h"
#include "gamepad_array_defs.h"
#include "gamepad_defs.h"

DEFINE_ARRAY(gamepad, Gamepad, Gamepad *)

[[gnu::nonnull]]
void gamepad_array_destroy(GamepadArray *array);
