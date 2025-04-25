#include "threads.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"

void mutex_destroy(pthread_mutex_t *const mutex) {
    assert(mutex != NULL);

    const int return_code = pthread_mutex_destroy(mutex);
    if (return_code != 0) {
        log_errorf("failed to destroy mutex: %s", strerror(return_code));
        exit(EXIT_FAILURE);
    }
}
