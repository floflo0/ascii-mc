#include "log.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_OUT stderr

static char *program_name = NULL;

void logger_init(const char *const restrict new_program_name) {
    assert(new_program_name != NULL);
    const size_t program_name_size = strlen(new_program_name) + 1;
    program_name = malloc(sizeof(*program_name) * program_name_size);
    if (program_name == NULL) {
        fprintf(stderr, "%s: error: failed to allocate memory: %s\n",
                new_program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    memcpy(program_name, new_program_name, program_name_size);
}

void log_quit(void) {
    assert(program_name != NULL && "logger module hasn't been initialized");
    free(program_name);
}

void _log_errorf(location_param const char *const restrict format, ...) {
    assert(program_name && "logger module hasn't been initialized");
    assert(format != NULL);
    fprintf(stderr, "%s ", program_name);
#ifndef PROD
    assert(file != NULL);
    assert(function_name != NULL);
    fprintf(stderr, "%s:%lu: %s:", file, line, function_name);
#endif
    fprintf(stderr, "error: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

void _log_errorf_errno(location_param const char *const restrict format, ...) {
    assert(program_name && "logger module hasn't been initialized");
    assert(format != NULL);
    fprintf(stderr, "%s:", program_name);
#ifndef PROD
    assert(file != NULL);
    assert(function_name != NULL);
    fprintf(stderr, "%s:%lu: %s:", file, line, function_name);
#endif
    fprintf(stderr, "error: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, ": %s\n", strerror(errno));
}

#ifndef PROD
void _log_debugf(const char *const restrict file, const size_t line,
                 const char *const restrict function_name,
                 const char *const restrict format, ...) {
    assert(program_name && "logger module hasn't been initialized");
    assert(file != NULL);
    assert(format != NULL);
    fprintf(DEBUG_OUT, "%s: %s:%lu: %s: debug: ", program_name, file, line,
            function_name);
    va_list args;
    va_start(args, format);
    vfprintf(DEBUG_OUT, format, args);
    va_end(args);
    fputc('\n', DEBUG_OUT);
}
#endif
