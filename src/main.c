#include <assert.h>
#include <libgen.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "log.h"
#include "xorshift.h"

int main(const int argc, char *argv[]) {
    assert(argc > 0);
    (void)argc;

    logger_init(basename(*argv++));

    if (*argv) {
        log_errorf("too many arguments");
        exit(EXIT_FAILURE);
    }

    xorshift32_set_seed(time(NULL));

    game_init(1, xorshift32());
    game_run();
    game_quit();

    log_quit();

    return EXIT_SUCCESS;
}
