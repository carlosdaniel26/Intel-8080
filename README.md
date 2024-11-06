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


# Intel 8080 Instructions Checklist

## Arithmetic Instructions
- [ ] **ADD r** - Add register `r` to accumulator.
- [ ] **ADI data** - Add immediate data to accumulator.
- [ ] **ADC r** - Add register `r` and carry to accumulator.
- [ ] **ACI data** - Add immediate data and carry to accumulator.
- [ ] **SUB r** - Subtract register `r` from accumulator.
- [ ] **SUI data** - Subtract immediate data from accumulator.
- [ ] **SBB r** - Subtract register `r` and borrow from accumulator.
- [ ] **SBI data** - Subtract immediate data and borrow from accumulator.
- [ ] **INR r** - Increment register `r`.
- [X] **DCR r** - Decrement register `r`.
- [X] **INX rp** - Increment register pair `rp`.
- [ ] **DCX rp** - Decrement register pair `rp`.
- [ ] **DAD rp** - Add register pair `rp` to `HL` register pair.

## Data Transfer Instructions
- [ ] **MOV r1, r2** - Move data from `r2` to `r1`.
- [X] **MVI r, data** - Move immediate data to register `r`.
- [X] **LXI rp, data** - Load immediate data into register pair `rp`.
- [ ] **LDA addr** - Load data from memory address into accumulator.
- [ ] **STA addr** - Store accumulator data at memory address.
- [ ] **LHLD addr** - Load `HL` pair directly from memory.
- [ ] **SHLD addr** - Store `HL` pair directly into memory.
- [X] **LDAX rp** - Load accumulator from address in register pair `rp`.
- [ ] **STAX rp** - Store accumulator into address in register pair `rp`.

## Branching Instructions
- [X] **JMP addr** - Jump to memory address.
- [ ] **JC addr** - Jump if carry is set.
- [ ] **JNC addr** - Jump if carry is not set.
- [ ] **JZ addr** - Jump if zero flag is set.
- [X] **JNZ addr** - Jump if zero flag is not set.
- [ ] **JP addr** - Jump if positive.
- [ ] **JM addr** - Jump if minus.
- [ ] **CALL addr** - Call subroutine at memory address.
- [ ] **RET** - Return from subroutine.

## Logical Instructions
- [ ] **ANA r** - Logical AND with register `r`.
- [ ] **ANI data** - Logical AND with immediate data.
- [ ] **XRA r** - Logical XOR with register `r`.
- [ ] **XRI data** - Logical XOR with immediate data.
- [ ] **ORA r** - Logical OR with register `r`.
- [ ] **ORI data** - Logical OR with immediate data.
- [ ] **CPI data** - Compare immediate data with accumulator.

## Stack Instructions
- [ ] **PUSH rp** - Push register pair `rp` onto stack.
- [ ] **POP rp** - Pop from stack into register pair `rp`.
- [ ] **XTHL** - Exchange `HL` with top of stack.
- [ ] **SPHL** - Move `HL` to stack pointer.

## I/O and Machine Control Instructions
- [ ] **IN port** - Input data from specified port to accumulator.
- [ ] **OUT port** - Output data from accumulator to specified port.
- [X] **HLT** - Halt processor execution.
- [X] **NOP** - No operation.
