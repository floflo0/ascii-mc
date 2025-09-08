#!/usr/bin/python

import ctypes

ISALNUM: int = 0
ISDIGIT: int = 1
ISPUNCT: int = 2
ISSPACE: int = 3


def main() -> None:
    libc = ctypes.cdll.LoadLibrary('libc.so.6')
    print('#pragma once')
    print()
    print('const char ctype_lookup_table[] = {')
    for i in range(128):
        value = (
            bool(libc.isalnum(i)) << ISALNUM |
            bool(libc.isdigit(i)) << ISDIGIT |
            bool(libc.ispunct(i)) << ISPUNCT |
            bool(libc.isspace(i)) << ISSPACE
        )
        print(f'    [{hex(i)}] = {hex(value)}, /* {repr(chr(i))} */')
    print('};')


if __name__ == '__main__':
    main()
