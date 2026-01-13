#include <assert.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "js.h"

int ioctl([[maybe_unused]] const int fd, const unsigned long op, ...) {
    assert(fd == STDOUT_FILENO && "unsupported fd");
    assert(op == TIOCGWINSZ && "unsupported op");

    va_list args;
    va_start(args, op);
    struct winsize *const window_size = va_arg(args, struct winsize *);
    window_size->ws_col = JS_get_terminal_width();
    window_size->ws_row = JS_get_terminal_height();
    window_size->ws_xpixel = 0;
    window_size->ws_ypixel = 0;
    va_end(args);

    return 0;
}
