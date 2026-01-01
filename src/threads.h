#pragma once

#ifndef __wasm__

#include <assert.h>
#include <pthread.h>

[[gnu::nonnull]]
void mutex_destroy(pthread_mutex_t *const mutex);

[[gnu::nonnull]]
static inline void mutex_lock(pthread_mutex_t *const mutex) {
    assert(mutex != NULL);
    [[maybe_unused]] const int return_code = pthread_mutex_lock(mutex);
    assert(return_code == 0 && "mutex lock failed");
}

[[gnu::nonnull]]
static inline void mutex_unlock(pthread_mutex_t *const mutex) {
    assert(mutex != NULL);
    [[maybe_unused]] const int return_code = pthread_mutex_unlock(mutex);
    assert(return_code == 0 && "mutex unlock failed");
}

#else

#define mutex_lock(mutex)
#define mutex_unlock(mutex)

#endif
