# ASCII-MC

A simple minecraft clone that render in ASCII characters in the terminal.

## Features

- Requires a game controller (keyboard input is not supported)
- Runs even in a TTY
- Explore a nearly infinite world with plains, mountains, and deserts
- Place and break blocks in the environment

## Build

First you can configure the game from [config.h](./src/config.h). More
particularly, the character ratio of the font in your terminal, other wise, the
game may be squished.

You may also need to change the buttons code for your controller in
[controller.c](./src/controller.c).

Then, to build the project in release mode:

```sh
make BUILD_TYPE=release
```

## Usage

To run the game:

```sh
./main
```
## Development

In debug mode, the engine writes logs to standard error (stderr). These logs can
interfere with the game's terminal display, making it glitchy. To fix this, you
can redirect the logs to a file:

```sh
./main 2> logs.txt
```

You can then see the logs from an other terminal window:

```sh
tail -f logs.txt
```

### Tests

This project uses [Check](https://libcheck.github.io/check/) for unit testting.

To run tests:

```sh
make test
```

### Lint

This project uses clang-tidy for linting. Run:

```sh
make lint
```

### Format

This project uses clang-format for consistent code style. To format all files:

```sh
make format
```
