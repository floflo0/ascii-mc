#!/usr/bin/python

import itertools
import os.path
import sys

TEXTURE_SIZE: int = 16
EXIT_FAILURE: int = 1


COLOR_DARK_RED: int     = 0
COLOR_DARK_GREEN: int   = 1
COLOR_DARK_YELLOW: int  = 2
COLOR_DARK_BLUE: int    = 3
COLOR_DARK_MAGENTA: int = 4
COLOR_DARK_CYAN: int    = 5
COLOR_LIGHT_GREY: int   = 6
COLOR_DARK_GREY: int    = 7
COLOR_RED: int          = 8
COLOR_GREEN: int        = 9
COLOR_YELLOW: int       = 10
COLOR_BLUE: int         = 11
COLOR_MAGENTA: int      = 12
COLOR_CYAN: int         = 13
COLOR_WHITE: int        = 14


colors: dict[str, int] = {
    '170 0 0': COLOR_DARK_RED,
    '0 170 0': COLOR_DARK_GREEN,
    '170 85 0': COLOR_DARK_YELLOW,
    '0 0 170': COLOR_DARK_BLUE,
    '170 0 170': COLOR_DARK_MAGENTA,
    '0 170 170': COLOR_DARK_CYAN,
    '170 170 170': COLOR_LIGHT_GREY,
    '85 85 85': COLOR_DARK_GREY,
    '255 85 85': COLOR_RED,
    '85 255 85': COLOR_GREEN,
    '255 255 85': COLOR_YELLOW,
    '85 85 255': COLOR_BLUE,
    '255 85 255': COLOR_MAGENTA,
    '85 255 255': COLOR_CYAN,
    '255 255 255': COLOR_WHITE,
}


def except_token(token: str, excepted_token: str) -> None:
    if token != excepted_token:
        print(
            f'error: expected {repr(excepted_token)} but got {repr(token)}',
            file=sys.stderr
        )
        sys.exit(EXIT_FAILURE)


def parse_ppm(ppm_file_path: str) -> list[int]:
    pixels: list[int] = []
    with open(ppm_file_path, 'r', encoding='utf-8') as file:
        token_iter = filter(
            lambda line: line,
            itertools.chain.from_iterable(map(
                lambda line: line.split('#')[0].strip().split(),
                file.readlines()
            ))
        )
        except_token(next(token_iter), 'P3')
        except_token(next(token_iter), str(TEXTURE_SIZE))
        except_token(next(token_iter), str(TEXTURE_SIZE))
        except_token(next(token_iter), '255')
        for _ in range(TEXTURE_SIZE**2):
            r = next(token_iter)
            g = next(token_iter)
            b = next(token_iter)
            pixels.append(colors[f'{r} {g} {b}'])
    return pixels


def export_pixels_to_c(a: str, pixels: list[int]) -> None:
    variable_name: str = os.path.basename(a)[:-2] + '_texture'
    with open(a, 'w', encoding='utf-8') as file:
        file.write('#include "texture.h"\n')
        file.write('\n')
        file.write(
            f'const Texture {variable_name}[TEXTURE_SIZE * TEXTURE_SIZE] = {{\n'
        )
        for i in range(TEXTURE_SIZE):
            i *= TEXTURE_SIZE
            file.write(f"    {', '.join(map(
                hex,
                pixels[i:i + TEXTURE_SIZE]
            ))},\n")
        file.write('};\n')


def main() -> None:
    pixels: list[int] = parse_ppm(sys.argv[1])
    export_pixels_to_c(sys.argv[2], pixels)


if __name__ == '__main__':
    main()
