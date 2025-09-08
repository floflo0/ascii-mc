#ifndef __FILE_defined
#define __FILE_defined

#include <stddef.h>

#define FILE_BUFFER_SIZE 1024

typedef struct {
    size_t buffer_length;
    char buffer[FILE_BUFFER_SIZE];
    int fd;
} FILE;

#endif  // __FILE_defined
