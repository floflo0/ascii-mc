#pragma once

#include "array.h"
#include "controller_defs.h"

DEFINE_ARRAY(controller, Controller, Controller *)

void controller_array_destroy(ControllerArray *array) NONNULL();
