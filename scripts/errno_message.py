#!/usr/bin/python

import errno
import os


def main() -> None:
    print('#pragma once')
    print()
    print('#include <errno.h>')
    print()
    print('char *const errno_message[] = {')

    for code, name in errno.errorcode.items():
        print(f'    [{name}] = "{os.strerror(code)}",')

    print('};')


if __name__ == '__main__':
    main()
