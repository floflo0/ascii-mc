// clang-format off
// WARN: This include must be before stdio.h.
#include "file.h"
// clang-format on

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "js.h"

#define DEFAULT_DOUBLE_DECIMAL_PART_MAX_LENGTH 7
#define DOUBLE_MAX_DECIMAL_PART_LENGTH 10

#define INT_STRING_MAX_LENGTH 21
#define DOUBLE_STRING_MAX_LENGTH (312 + DOUBLE_MAX_DECIMAL_PART_LENGTH)
#define HEX_STRING_MAX_LENGTH 17

FILE stdout_file = {
    .fd = STDOUT_FILENO,
    .buffer = {0},
};
FILE *stdout = &stdout_file;

FILE stderr_file = {
    .fd = STDERR_FILENO,
    .buffer = {0},
};
FILE *stderr = &stderr_file;

static char *uint_to_string(uint64_t value, char dest[INT_STRING_MAX_LENGTH]) {
    char *output_ptr = &dest[INT_STRING_MAX_LENGTH - 1];
    *output_ptr = '\0';
    do {
        const uint32_t digit = value % 10;
        *(--output_ptr) = '0' + digit;
        value /= 10;
    } while (value);
    return output_ptr;
}

static char *int_to_string(int64_t value, char dest[INT_STRING_MAX_LENGTH]) {
    char *output_ptr = &dest[INT_STRING_MAX_LENGTH - 1];
    const bool is_negative = value < 0;
    if (is_negative) value = -value;
    *output_ptr = '\0';
    do {
        const uint32_t digit = value % 10;
        *(--output_ptr) = '0' + digit;
        value /= 10;
    } while (value);
    if (is_negative) {
        *(--output_ptr) = '-';
    }
    return output_ptr;
}

static char *hex_to_string(uint64_t value, char dest[HEX_STRING_MAX_LENGTH]) {
    char *output_ptr = &dest[HEX_STRING_MAX_LENGTH - 1];
    *output_ptr = '\0';
    do {
        const uint32_t digit = value % 16;
        *(--output_ptr) = digit < 10 ? ('0' + digit) : ('a' + digit - 10);
        value >>= 4;
    } while (value);
    return output_ptr;
}

static char *double_to_string(double value, char dest[DOUBLE_STRING_MAX_LENGTH],
                              int double_decimal_part_length) {
    assert(0 <= double_decimal_part_length &&
           double_decimal_part_length <= DOUBLE_MAX_DECIMAL_PART_LENGTH);
    const bool is_negative = value < 0;
    if (is_negative) value = -value;

    char *output_ptr = &dest[DOUBLE_STRING_MAX_LENGTH - 1];
    *output_ptr = '\0';

    if (double_decimal_part_length) {
        for (; double_decimal_part_length; --double_decimal_part_length) {
            const int digit = (int)fmod(
                value * pow(10.0f, double_decimal_part_length), 10.0f);
            *(--output_ptr) = '0' + digit;
        }
        *(--output_ptr) = '.';
    }

    value = trunc(value);
    do {
        const uint32_t digit = fmod(value, 10.0f);
        *(--output_ptr) = '0' + digit;
        value /= 10.0f;
    } while (value >= 1.0f);
    if (is_negative) {
        *(--output_ptr) = '-';
    }

    return output_ptr;
}

#define APPEND_STRING(string, append_char)                          \
    do {                                                            \
        if (string_min_length > 0) {                                \
            const int padding = string_min_length - strlen(string); \
            for (int i = 0; i < padding; ++i) {                     \
                const char chr = padding_char;                      \
                append_char;                                        \
                ++count;                                            \
            }                                                       \
        }                                                           \
        const char *string_ptr = string;                            \
        while (*string_ptr) {                                       \
            const char chr = *string_ptr++;                         \
            append_char;                                            \
            ++count;                                                \
        }                                                           \
    } while (0)

#define PRINTF_IMP(append_char)                                                \
    do {                                                                       \
        for (; *format; ++format) {                                            \
            if (*format != '%') {                                              \
                const char chr = *format;                                      \
                append_char;                                                   \
                ++count;                                                       \
                continue;                                                      \
            }                                                                  \
            int string_min_length = -1;                                        \
            char padding_char = ' ';                                           \
            int double_decimal_part_length =                                   \
                DEFAULT_DOUBLE_DECIMAL_PART_MAX_LENGTH;                        \
            ++format;                                                          \
        parse_format:                                                          \
            switch (*format) {                                                 \
                case '%': {                                                    \
                    const char chr = '%';                                      \
                    append_char;                                               \
                    ++count;                                                   \
                    break;                                                     \
                }                                                              \
                                                                               \
                case 'c': {                                                    \
                    const char chr = va_arg(ap, int);                          \
                    append_char;                                               \
                    ++count;                                                   \
                    break;                                                     \
                }                                                              \
                                                                               \
                case 's': {                                                    \
                    assert(padding_char == ' ' &&                              \
                           "unsupported padding_char for %s");                 \
                    const char *string = va_arg(ap, char *);                   \
                    APPEND_STRING(string, append_char);                        \
                    break;                                                     \
                }                                                              \
                                                                               \
                case 'd': {                                                    \
                    const int value = va_arg(ap, int);                         \
                    char string[INT_STRING_MAX_LENGTH];                        \
                    const char *number_string = int_to_string(value, string);  \
                    APPEND_STRING(number_string, append_char);                 \
                    break;                                                     \
                }                                                              \
                                                                               \
                case 'x': {                                                    \
                    char string[HEX_STRING_MAX_LENGTH];                        \
                    const char *hex_string =                                   \
                        hex_to_string(va_arg(ap, unsigned int), string);       \
                    APPEND_STRING(hex_string, append_char);                    \
                    break;                                                     \
                }                                                              \
                                                                               \
                case 'u': {                                                    \
                    const unsigned int value = va_arg(ap, unsigned int);       \
                    char string[INT_STRING_MAX_LENGTH];                        \
                    const char *number_string = uint_to_string(value, string); \
                    APPEND_STRING(number_string, append_char);                 \
                    break;                                                     \
                }                                                              \
                                                                               \
                case 'l':                                                      \
                    ++format;                                                  \
                    if (*format == 'u') {                                      \
                        const unsigned long value = va_arg(ap, unsigned long); \
                        char string[INT_STRING_MAX_LENGTH];                    \
                        const char *number_string =                            \
                            uint_to_string(value, string);                     \
                        APPEND_STRING(number_string, append_char);             \
                        break;                                                 \
                    }                                                          \
                    if (*format == 'd') {                                      \
                        const long value = va_arg(ap, long);                   \
                        char string[INT_STRING_MAX_LENGTH];                    \
                        const char *number_string =                            \
                            int_to_string(value, string);                      \
                        APPEND_STRING(number_string, append_char);             \
                        break;                                                 \
                    }                                                          \
                    assert(false && "unreachable");                            \
                                                                               \
                case 'p': {                                                    \
                    assert(padding_char == ' ' &&                              \
                           "unsupported padding_char for %s");                 \
                    char chr = '0';                                            \
                    append_char;                                               \
                    ++count;                                                   \
                    chr = 'x';                                                 \
                    append_char;                                               \
                    ++count;                                                   \
                    char string[HEX_STRING_MAX_LENGTH];                        \
                    const char *hex_string =                                   \
                        hex_to_string((uint64_t)va_arg(ap, void *), string);   \
                    APPEND_STRING(hex_string, append_char);                    \
                    break;                                                     \
                }                                                              \
                                                                               \
                case 'f': {                                                    \
                    double value = va_arg(ap, double);                         \
                    if (isnan(value)) {                                        \
                        char string[] = "nan";                                 \
                        APPEND_STRING(string, append_char);                    \
                        break;                                                 \
                    }                                                          \
                    if (isinf(value)) {                                        \
                        if (value > 0) {                                       \
                            char string[] = "inf";                             \
                            APPEND_STRING(string, append_char);                \
                        } else {                                               \
                            char string[] = "-inf";                            \
                            APPEND_STRING(string, append_char);                \
                        }                                                      \
                        break;                                                 \
                    }                                                          \
                    char string[DOUBLE_STRING_MAX_LENGTH];                     \
                    const char *number_string = double_to_string(              \
                        value, string, double_decimal_part_length);            \
                    APPEND_STRING(number_string, append_char);                 \
                    break;                                                     \
                }                                                              \
                                                                               \
                case '.':                                                      \
                    ++format;                                                  \
                    double_decimal_part_length = 0;                            \
                    if (isdigit(*format)) {                                    \
                        do {                                                   \
                            double_decimal_part_length =                       \
                                double_decimal_part_length * 10 + *format -    \
                                '0';                                           \
                            ++format;                                          \
                        } while (isdigit(*format));                            \
                        assert(*format == 'f');                                \
                        goto parse_format;                                     \
                    }                                                          \
                    assert(false && "unreachable");                            \
                                                                               \
                default:                                                       \
                    if (isdigit(*format)) {                                    \
                        string_min_length = 0;                                 \
                        if (*format == '0') {                                  \
                            padding_char = '0';                                \
                            ++format;                                          \
                            assert(isdigit(*format));                          \
                        }                                                      \
                        do {                                                   \
                            string_min_length =                                \
                                string_min_length * 10 + *format - '0';        \
                            ++format;                                          \
                        } while (isdigit(*format));                            \
                        goto parse_format;                                     \
                    }                                                          \
                    assert(false && "unreachable");                            \
            }                                                                  \
        }                                                                      \
    } while (0)

int vsnprintf(char *str, size_t size, const char *restrict format, va_list ap) {
#define APPEND_CHAR                      \
    do {                                 \
        str[count] = chr;                \
        if ((size_t)count == size - 1) { \
            str[count] = '\0';           \
            return count;                \
        }                                \
    } while (0)

    int count = 0;
    PRINTF_IMP(APPEND_CHAR);
    str[count] = '\0';
    return count;

#undef APPEND_CHAR
}

int snprintf(char *str, size_t size, const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    const int return_value = vsnprintf(str, size, format, args);
    va_end(args);
    return return_value;
}

int vfprintf(FILE *restrict stream, const char *restrict format, va_list ap) {
    int count = 0;
    PRINTF_IMP(fputc(chr, stream));
    return count;
}

int fprintf(FILE *restrict stream, const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    const int return_value = vfprintf(stream, format, args);
    va_end(args);
    return return_value;
}

int printf(const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    const int return_value = vfprintf(stdout, format, args);
    va_end(args);
    return return_value;
}

[[gnu::nonnull]]
static void file_flush(FILE *self) {
    assert(self != NULL);
    if (self->fd == STDOUT_FILENO) {
        JS_console_log(self->buffer);
        self->buffer_length = 0;
        return;
    }

    if (self->fd == STDERR_FILENO) {
        JS_console_error(self->buffer);
        self->buffer_length = 0;
        return;
    }

    assert(false && "unsupported stream");
}

int fputc(int c, FILE *stream) {
    assert(stream != NULL);
    assert(stream->buffer_length < FILE_BUFFER_SIZE - 1);
    if (c == '\n') {
        stream->buffer[stream->buffer_length] = '\0';
        file_flush(stream);
    } else {
        stream->buffer[stream->buffer_length++] = c;
    }
    if (stream->buffer_length == FILE_BUFFER_SIZE - 1) {
        stream->buffer[FILE_BUFFER_SIZE - 1] = '\0';
        file_flush(stream);
    }
    return c;
}
