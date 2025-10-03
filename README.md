<h1 align="center">
  <a href="https://wikipedia.org/wiki/Intel_8080">Intel 8080 Emulator</a>
</h1>


<p align="center">
  <a href="https://pt.wikipedia.org/wiki/Intel_8080">
    <img src="https://github.com/user-attachments/assets/1a9050c6-51a8-47bf-a0f6-b0eed3034a56" alt="image">
  </a>
</p>


This project is an implementation of an Intel 8080 CPU emulator, developed in pure C, using SDL2 to render the video memory buffer in a graphical window.

## Features

- Full emulation of the Intel 8080 CPU.
- Uses SDL2 to display the video and memory buffer.
- Loads and executes binaries compatible with the Intel 8080.

<p align="center">
  <img src="https://github.com/user-attachments/assets/2173af3f-8b79-4e62-a6b7-e28269d9dba8" alt="image">
</p>


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

### Explanation
```
- build/: Where the compiled object files and the final emulator binary (`main`) live.
- include/: Header files with declarations, defines, and interfaces.
- rom/: Contains your assembly code, compiled binaries, and test CPU programs.
- src/: All your C source files making the emulator tick.
- gdb_script.gdb: Script to help automate debugging sessions with GDB.
- Makefile: Handles compiling and linking everything in the right order.
```
---
