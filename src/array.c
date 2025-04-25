#include "array.h"

void array_destroy(Array *const self) {
    assert(self != NULL);
    assert(self->array != NULL);
    free(self->array);
    free(self);
}
