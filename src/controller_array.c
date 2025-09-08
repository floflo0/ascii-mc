#include "controller_array.h"

#include "controller.h"

ARRAY_IMPLEMENTATION(controller, Controller, Controller *)

void controller_array_destroy(ControllerArray *const array) {
    assert(array != NULL);
    for (size_t i = 0; i < array->length; ++i) {
        Controller *const controller = array->array[i];
        assert(controller != NULL);
        controller_destroy(controller);
    }
    array_destroy((Array *)array);
}
