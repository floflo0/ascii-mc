#include "controller_array.h"

#include "controller.h"

ARRAY_IMPLEMENTATION(controller, Controller, Controller *)

void controller_array_destroy(ControllerArray *const array) {
    assert(array != NULL);
    for (size_t i = 0; i < array->length; ++i) {
        assert(array->array[i] != NULL);
        controller_destroy(array->array[i]);
    }
    array_destroy((Array *)array);
}
