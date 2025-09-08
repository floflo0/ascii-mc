#include "wasm.h"

#include <assert.h>
#include <stdlib.h>

void run_callback(void (*callback)(void)) {
    assert(callback != NULL);
    callback();
}

void run_callback_int(void (*callback)(const int), const int param) {
    assert(callback != NULL);
    callback(param);
}

void run_callback_ptr(void (*callback)(void *const), void *const param) {
    assert(callback != NULL);
    callback(param);
}

void wasm_main(void) {
    char *argv[] = {"ascii-mc", NULL};
    main((sizeof(argv) / sizeof(*argv)) - 1, argv);

    exit(EXIT_SUCCESS);
}
