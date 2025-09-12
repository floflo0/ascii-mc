# ASCII-MC

A simple Minecraft clone that render in ASCII characters in the terminal.

## Features

- Requires a game controller (keyboard input is not supported)
- Runs even in a TTY
- Explore a nearly infinite world with plains, mountains, and deserts
- Place and break blocks in the environment
- Demo in web browser: <https://floflo0.github.io/ascii-mc/>

## Web browser Demo

The game can be compile to WebAssembly in order to run in web browser. However,
this version of the game is very slow and if you really want to test the game
properly, I really encourage you to test the native version.

The game is available here: <https://floflo0.github.io/ascii-mc/>

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

#### WebAssembly

```sh
make wasm BUILD_TYPE=release
cd wasm
npm install
npm run build
```

## Usage

To run the game in your terminal or TTY:

```sh
build/ascii-mc
```

Alternatively you can launch the game with a custom alacritty config that
imitate a Linux TTY:

```sh
alacritty --config-file alacritty.toml --command build/ascii-mc
```

## Development

In debug mode, the engine writes logs to standard error (stderr). These logs can
interfere with the game's terminal display, making it glitchy. To fix this, you
can redirect the logs to a file:

```sh
build/ascii-mc 2> logs.txt
```

You can then see the logs from an other terminal window:

```sh
tail -f logs.txt
```

### Tests

This project uses [Check](https://libcheck.github.io/check/) for unit testing.

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
