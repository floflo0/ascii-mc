#include <assert.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include "js.h"

time_t time(time_t *timer) {
    const uint64_t now = JS_Date_now() / 1000;
    if (timer != NULL) {
        *timer = now;
    }
    return now;
}

int clock_gettime(clockid_t clockid, struct timespec *tp) {
    assert(tp != NULL);

    if (clockid != CLOCK_MONOTONIC_RAW) {
        errno = EINVAL;
        return -1;
    }

    const double now_miliseconds = JS_performance_now();
    tp->tv_sec = now_miliseconds / 1000.0;
    tp->tv_nsec = fmod(now_miliseconds, 1000.0) * 1000000.0;

    return 0;
}
