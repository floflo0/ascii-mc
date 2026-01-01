#pragma once

/**
 * Command line arguments parser module.
 */

// FLAG(
//     variable_name,
//     name,
//     short_name,
//     description
// )

// LONG_FLAG(
//     variable_name,
//     name,
//     description
// )

// FLAG_WITH_PARAM(
//     variable_name,
//     name,
//     short_name,
//     arg_name,
//     description
// )

#include <stdbool.h>
#include <stdio.h>

#include "args_config.h"

typedef struct {
#define FLAG(variable_name, name, short_name, description) bool variable_name;
#define LONG_FLAG(variable_name, name, description) bool variable_name;
#define FLAG_WITH_PARAM(variable_name, name, short_name, arg_name, \
                        description)                               \
    const char *variable_name;
    ARGS_FLAGS
#undef FLAG
#undef LONG_FLAG
#undef FLAG_WITH_PARAM
} Args;

/**
 * Print the usage of the program.
 *
 * \param program_name The name of the program being executed to print in the
 *                     usage.
 * \param stream The output stream.
 */
[[gnu::nonnull]]
void print_usage(const char *const restrict program_name,
                 FILE *const restrict stream);

/**
 * Print the help message for the program.
 *
 * \param program_name The name of the program being executed.
 */
[[gnu::nonnull]]
void print_help(const char *const program_name);

/**
 * Parse command-line arguments.
 *
 * \param args The structure to store parsed arguments.
 * \param argv The arguments passed to the program.
 * \param program_name The name of the program being executed.
 *
 * \return true if the arguments were successfully parsed, or false on failure.
 */
[[gnu::nonnull]]
bool args_parse(Args *const restrict args, char *const restrict argv[],
                const char *const restrict program_name);
