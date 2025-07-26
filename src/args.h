#pragma once

#include <stdbool.h>
#include <stdio.h>

#define NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define RETURNS_NONNULL __attribute__((returns_nonnull))

#define HELP_SPACING "20"

#define ARGS_FLAGS                                                        \
    FLAG(help, "help", h, "show this help message and exit")              \
    FLAG(version, "version", v, "show program's version number and exit") \
    FLAG_WITH_PARAM(players, "players", p, PLAYERS, "TODO")               \
    FLAG_WITH_PARAM(world_seed, "world-seed", s, SEED, "TODO")

typedef struct {
#define FLAG(variable_name, name, short_name, description) bool variable_name;
#define FLAG_WITH_PARAM(variable_name, name, short_name, arg_name, \
                        description)                               \
    const char *variable_name;
    ARGS_FLAGS
#undef FLAG
#undef FLAG_WITH_PARAM
} Args;

/**
 * Print the usage of the program.
 *
 * \param program_name The name of the program being executed to print in the
 *                     usage.
 * \param stream The output stream.
 */
void print_usage(const char *const restrict program_name,
                 FILE *const restrict stream) NONNULL();

/**
 * Print the help message for the program.
 *
 * \param program_name The name of the program being executed.
 */
void print_help(const char *const program_name) NONNULL();

/**
 * Parse command-line arguments.
 *
 * \param args The structure to store parsed arguments.
 * \param argv The arguments passed to the program.
 * \param program_name The name of the program being executed.
 *
 * \return true if the arguments were successfully parsed, or false on failure.
 */
bool args_parse(Args *const restrict args, char *const restrict argv[],
                const char *const restrict program_name) NONNULL();
