#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errno_message.h"

#define STRTOK_TOKEN_CAPACITY 1024
#define ERRNO_MAX EHWPOISON

void *memcpy(void *restrict dest, const void *restrict src, const size_t n) {
    assert(dest != NULL);
    assert(src != NULL);
    for (size_t i = 0; i < n; ++i) {
        ((char *)dest)[i] = ((const char *)src)[i];
    }
    return dest;
}

void *memset(void *s, const int c, const size_t n) {
    assert(s != NULL);
    void *const original_pointer = s;
    void *const end_pointer = s + n;
    while (s < end_pointer) *(char *)s++ = c;
    return original_pointer;
}

int memcmp(const void *const s1, const void *const s2, const size_t size) {
    assert(s1 != NULL);
    assert(s2 != NULL);
    const char *const s1_char = (const char *)s1;
    const char *const s2_char = (const char *)s2;
    for (size_t i = 0; i < size; ++i) {
        if (s1_char[i] != s2_char[i]) return s1_char[i] - s2_char[i];
    }
    return 0;
}

size_t strlen(const char *s) {
    assert(s != NULL);
    size_t size = 0;
    while (s[size]) ++size;
    return size;
}

char *strcpy(char *restrict dst, const char *restrict src) {
    while (*src) *dst++ = *src++;
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    assert(s1 != NULL);
    assert(s2 != NULL);
    for (size_t i = 0;; ++i) {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        if (s1[i] == '\0') break;
    }
    return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    assert(s1 != NULL);
    assert(s2 != NULL);
    for (size_t i = 0; i < n; ++i) {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        if (s1[i] == '\0') break;
    }
    return 0;
}

char *strtok(char *const restrict str, const char *const restrict delim) {
    assert(delim != NULL);
    static char *string = NULL;
    static char token[STRTOK_TOKEN_CAPACITY];
    if (str != NULL) {
        string = str;
    }

    assert(string != NULL);

    if (*string == '\0') return NULL;

    const char *delim_ptr = delim;
    size_t token_length = 0;
    ;
    for (; *string; ++string) {
        if (*string == *delim_ptr) {
            ++delim_ptr;
            if (*delim_ptr == '\0') {
                ++string;
                goto return_;
            }
            continue;
        }

        const size_t foo = delim_ptr - delim_ptr;
        for (size_t i = 0; i < foo; ++i) {
            token[token_length++] = delim[i];
            assert(token_length < STRTOK_TOKEN_CAPACITY);
        }
        token[token_length++] = *string;
        assert(token_length < STRTOK_TOKEN_CAPACITY);
    }

return_:
    token[token_length] = '\0';
    return token;
}

char *strdup(const char *s) {
    assert(s != NULL);
    char *string = malloc((strlen(s) + 1) * sizeof(char));
    if (string == NULL) return NULL;
    strcpy(string, s);
    return string;
}

char *strerror(int errnum) {
    static char unknown_error_message[26];
    if (errnum < 0 || errnum > ERRNO_MAX) {
        snprintf(unknown_error_message, sizeof(unknown_error_message),
                 "Unknown error %d", errnum);
        return unknown_error_message;
    }
    return errno_message[errnum];
}
