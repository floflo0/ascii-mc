#pragma once

#define ARGS_DESCRIPTION "TODO: description"

#define ARGS_FLAGS                                                        \
    FLAG(help, "help", h, "show this help message and exit")              \
    FLAG(version, "version", v, "show program's version number and exit") \
    FLAG_WITH_PARAM(players, "players", p, PLAYERS, "TODO")               \
    FLAG_WITH_PARAM(world_seed, "world-seed", s, SEED, "TODO")            \
    FLAG(force_tty, "tty", t, "force tty mode")                           \
    LONG_FLAG(force_no_tty, "no-tty", "force disable tty mode")

#define ARGS_HELP_SPACING "20"
