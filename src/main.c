#include <assert.h>
#include <libgen.h>
#include <stdlib.h>
#include <time.h>

#include "args.h"
#include "game.h"
#include "log.h"
#include "utils.h"
#include "xorshift.h"

#ifndef VERSION
#define VERSION "0.0.0"
#endif

int main([[maybe_unused]] const int argc, char *argv[]) {
    assert(argc > 0);

    const char *const program_name = basename(*argv++);
    assert(program_name != NULL);

    logger_init(program_name);

    Args args = {0};
    if (!args_parse(&args, argv, program_name)) return EXIT_FAILURE;

    if (args.help) {
        print_help(program_name);
        return EXIT_FAILURE;
    }

    if (args.version) {
        printf("%s " VERSION "\n", program_name);
        return EXIT_SUCCESS;
    }

    int8_t number_players = 1;
    if (args.players != NULL) {
        int number_players_from_args;
        if (!parse_int(args.players, &number_players_from_args)) {
            log_errorf("invalid number of players: '%s'", args.players);
            return EXIT_FAILURE;
        }
        if (number_players_from_args <= 0 || number_players_from_args > 4) {
            log_errorf("number of players must be between 1 and 4");
            return EXIT_FAILURE;
        }
        number_players = number_players_from_args;
    }

    uint32_t world_seed;
    if (args.world_seed == NULL) {
        xorshift32_set_seed(time(NULL));
        world_seed = xorshift32();
    } else if (!parse_uint32(args.world_seed, &world_seed)) {
        log_errorf("invalid world seed: '%s'", args.world_seed);
        return EXIT_FAILURE;
    }

    game_init(number_players, world_seed, args.force_tty, args.force_no_tty);
    game_run();
    game_quit();

    log_quit();

    return EXIT_SUCCESS;
}
