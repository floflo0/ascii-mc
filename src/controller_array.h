#pragma once

#include "array.h"
#include "controller_array_defs.h"
#include "controller_defs.h"

DEFINE_ARRAY(controller, Controller, Controller *)

[[gnu::nonnull]]
void controller_array_destroy(ControllerArray *array);
