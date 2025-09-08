#pragma once

#ifndef __wasm__

#include <assert.h>
#include <pthread.h>

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

void mutex_destroy(pthread_mutex_t *const mutex) NONNULL();

static inline void mutex_lock(pthread_mutex_t *const mutex) NONNULL();

static inline void mutex_lock(pthread_mutex_t *const mutex) {
    assert(mutex != NULL);
    [[maybe_unused]] const int return_code = pthread_mutex_lock(mutex);
    assert(return_code == 0 && "mutex lock failed");
}

static inline void mutex_unlock(pthread_mutex_t *const mutex) NONNULL();

static inline void mutex_unlock(pthread_mutex_t *const mutex) {
    assert(mutex != NULL);
    [[maybe_unused]] const int return_code = pthread_mutex_unlock(mutex);
    assert(return_code == 0 && "mutex unlock failed");
}

#else

#define mutex_lock(mutex)
#define mutex_unlock(mutex)

#endif
