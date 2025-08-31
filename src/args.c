#include "args.h"

#include <string.h>

#include "log.h"

#define CHAR(c) #c[0]

void print_usage(const char *const restrict program_name,
                 FILE *const restrict stream) {
    fprintf(stream,
            "usage: %s "
#define FLAG(variable_name, name, short_name, description) "[-" #short_name "] "
#define LONG_FLAG(variable_name, name, description)
#define FLAG_WITH_PARAM(variable_name, name, short_name, arg_name, \
                        description)                               \
    "[-" #short_name " " #arg_name "] "
            ARGS_FLAGS
#undef FLAG
#undef LONG_FLAG
#undef FLAG_WITH_PARAM
            "\n",
            program_name);
}

void print_help(const char *const program_name) {
    print_usage(program_name, stdout);
    printf(
        "\n"
        ARGS_DESCRIPTION "\n"
        "\n"
        "options:\n"
#define FLAG(variable_name, name, short_name, description) \
    "    -" #short_name ", --%-" ARGS_HELP_SPACING "s " description "\n"
#define LONG_FLAG(variable_name, name, description) \
    "        --%-" ARGS_HELP_SPACING "s " description "\n"
#define FLAG_WITH_PARAM(variable_name, name, short_name, arg_name, \
                        description)                               \
    FLAG(variable_name, name, short_name, description)
        ARGS_FLAGS
#undef FLAG
#undef LONG_FLAG
#undef FLAG_WITH_PARAM

#define FLAG(variable_name, name, short_name, description) , name
#define LONG_FLAG(variable_name, name, description) , name
#define FLAG_WITH_PARAM(variable_name, name, short_name, arg_name, \
                        description)                               \
    , name " " #arg_name
            ARGS_FLAGS
#undef FLAG
#undef LONG_FLAG
#undef FLAG_WITH_PARAM
    );
}

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
                const char *const restrict program_name) {
    for (; *argv; ++argv) {
        if ((*argv)[0] == '-' && (*argv)[1]) {
            if ((*argv)[1] == '-') {
#define FLAG(variable_name, name, short_name, description) \
    LONG_FLAG(variable_name, name, description)

#define LONG_FLAG(variable_name, name, description)                          \
    {                                                                        \
        const char tested_arg[] = "--" name;                                 \
        const size_t tested_arg_length = strlen(tested_arg);                 \
        if (strncmp(*argv, tested_arg, tested_arg_length) == 0) {            \
            if ((*argv)[tested_arg_length] == '\0') {                        \
                args->variable_name = true;                                  \
                continue;                                                    \
            }                                                                \
            if ((*argv)[tested_arg_length] == '=') {                         \
                print_usage(program_name, stderr);                           \
                log_errorf("option '--" name "' doesn't allow an argument"); \
                return false;                                                \
            }                                                                \
        }                                                                    \
    }

#define FLAG_WITH_PARAM(variable_name, name, short_name, arg_name,     \
                        description)                                   \
    {                                                                  \
        const char tested_arg[] = "--" name;                           \
        const size_t tested_arg_length = strlen(tested_arg);           \
        if (strncmp(*argv, tested_arg, tested_arg_length) == 0) {      \
            if ((*argv)[tested_arg_length] == '\0') {                  \
                ++argv;                                                \
                if (*argv != NULL) {                                   \
                    args->variable_name = *argv;                       \
                    continue;                                          \
                }                                                      \
            } else if ((*argv)[tested_arg_length] == '=') {            \
                args->variable_name = &(*argv)[tested_arg_length + 1]; \
                continue;                                              \
            }                                                          \
            print_usage(program_name, stderr);                         \
            log_errorf("option '--" name "' requires an argument");    \
            return false;                                              \
        }                                                              \
    }

                ARGS_FLAGS

#undef FLAG
#undef LONG_FLAG
#undef FLAG_WITH_PARAM

                print_usage(program_name, stderr);
                log_errorf("unrecognized option: '%s'", *argv);
                return false;
            } else {
                for (size_t i = 1; (*argv)[i]; ++i) {
#define FLAG(variable_name, name, short_name, description) \
    if ((*argv)[i] == CHAR(short_name)) {                  \
        args->variable_name = true;                        \
        continue;                                          \
    }

#define LONG_FLAG(variable_name, name, description)

#define FLAG_WITH_PARAM(variable_name, name, short_name, arg_name, \
                        description)                               \
    if ((*argv)[i] == CHAR(short_name)) {                          \
        if ((*argv)[i + 1]) {                                      \
            args->variable_name = &(*argv)[i + 1];                 \
            break;                                                 \
        }                                                          \
        ++argv;                                                    \
        if (*argv != NULL) {                                       \
            args->variable_name = *argv;                           \
            break;                                                 \
        }                                                          \
        print_usage(program_name, stderr);                         \
        log_errorf("option requires an argument -- " #short_name); \
        return false;                                              \
    }

                    ARGS_FLAGS

#undef FLAG
#undef LONG_FLAG
#undef FLAG_WITH_PARAM

                    print_usage(program_name, stderr);
                    log_errorf("invalid option -- '%c'", (*argv)[i]);
                    return false;
                }
                continue;
            }
        }

        print_usage(program_name, stderr);
        log_errorf("unrecognized arguments: '%s'", *argv);
        return false;
    }

    return true;
}
