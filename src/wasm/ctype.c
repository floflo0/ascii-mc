#include <assert.h>
#ifndef __NO_CTYPE
#define __NO_CTYPE
#endif
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

#include "ctype_lookup_table.h"

#define ISALNUM 0
#define ISDIGIT 1
#define ISPUNCT 2
#define ISSPACE 3

int isalnum(const int chr) {
    if (chr < 0 || chr >= 128) return 0;
    return ctype_lookup_table[chr] & (1 << ISALNUM);
}

int isdigit(const int chr) {
    if (chr < 0 || chr >= 128) return 0;
    return ctype_lookup_table[chr] & (1 << ISDIGIT);
}

int ispunct(const int chr) {
    if (chr < 0 || chr >= 128) return 0;
    return ctype_lookup_table[chr] & (1 << ISPUNCT);
}

int isspace(const int chr) {
    if (chr < 0 || chr >= 128) return 0;
    return ctype_lookup_table[chr] & (1 << ISSPACE);
}
