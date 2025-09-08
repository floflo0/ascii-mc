#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>

#include "js.h"

ssize_t read(const int fd, void *const buf, const size_t count) {
    if (count == 0) return 0;

    if (fd != STDIN_FILENO) {
        errno = EBADF;
        return -1;
    }

    if (buf == NULL) {
        errno = EFAULT;
        return -1;
    }

    for (size_t i = 0; i < count; ++i) {
        const char chr = JS_read_char();
        if (chr == -1) return i;
        ((char *)buf)[i] = chr;
    }

    return count;
}

ssize_t write(int fd, const void *buf, size_t count) {
    if (fd != STDOUT_FILENO) {
        errno = EBADF;
        return -1;
    }

    if (buf == NULL) {
        errno = EFAULT;
        return -1;
    }

    JS_write(buf, count);

    return count;
}

void _exit(int status) {
    JS_exit(status);
}

int usleep(const useconds_t usec) {
    JS_usleep(usec);
    return 0;
}

char *ttyname(int fd) {
    if (fd == STDOUT_FILENO) return "/dev/pts/1";
    return "";
}
