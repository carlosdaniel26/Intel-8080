# [Intel 8080 Emulator](https://pt.wikipedia.org/wiki/Intel_8080)

[![image](https://github.com/user-attachments/assets/1a9050c6-51a8-47bf-a0f6-b0eed3034a56)](https://pt.wikipedia.org/wiki/Intel_8080)

This project is an implementation of an Intel 8080 CPU emulator, developed in pure C, using SDL2 to render the video memory buffer in a graphical window.

## Features

- Full emulation of the Intel 8080 CPU.
- Uses SDL2 to display the video and memory buffer.
- Loads and executes binaries compatible with the Intel 8080.

## Dependencies

To compile and run the emulator, ensure the following tools are installed:

- **GCC** (version 10.2.1)
- **GNU Make** (version 4.3)
- **SDL2** (version 2.0.14)

### Installing dependencies on Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential libsdl2-dev
```

### Installing dependencies on Arch Linux:
```
sudo pacman -S base-devel sdl2
```

### Installing dependencies on MacOS (Homebrew):
```
brew install sdl2
```

### Compilation and execution:
```
make all
make run
```
or
```
make debug
```

## Project Structure
- src/: Emulator source code.
- Makefile: Build automation file.
- roms/: Directory for binary program files to be emulated.
- build/: Directory for final binary program.
