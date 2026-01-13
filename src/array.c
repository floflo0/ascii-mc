#include "array.h"

#include <stdlib.h>

void array_destroy(const Array *const self) {
    assert(self != NULL);
    assert(self->array != NULL);
    free(self->array);
}
