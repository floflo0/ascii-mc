#include "wasm.h"

#include <assert.h>
#include <stdlib.h>

void run_callback(void (*callback)(void)) {
    assert(callback != NULL);
    callback();
}

void run_callback_uint32(void (*callback)(const uint32_t),
                         const uint32_t param) {
    assert(callback != NULL);
    callback(param);
}

void wasm_main(void) {
    char *argv[] = {"ascii-mc", NULL};
    malloc_init();
    main((sizeof(argv) / sizeof(*argv)) - 1, argv);

    exit(EXIT_SUCCESS);
}
