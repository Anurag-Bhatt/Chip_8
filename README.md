# Chip-8 Emulator
This project is a Chip-8 emulator written in C++ that uses SDL2 for graphics and input handling. It allows you to run classic Chip-8 ROMs and explore retro computing and gaming.
## Features
- **Chip-8 Emulation:** Implements many of the Chip-8 opcodes (arithmetic, logic, drawing, timers, etc.).
- **Graphical Output:** Uses SDL2 to render a 64×32 monochrome display (scaled up for modern screens).
- **Keyboard Input:** Maps a QWERTY keyboard to the Chip-8's 16-key hexadecimal keypad.
- **Debug Logging:** Displays debug information about opcode execution (optional for production builds).

## Dependencies
- **C++17** (or later)
- **SDL2**

### Installing SDL2 on Ubuntu
To install SDL2 development libraries, run:
```
sudo apt-get update
sudo apt-get install libsdl2-dev
```
# Building the Project
A Makefile is provided. To compile the project, simply run:

```
make
make clean // To clean up artifacts
```
### Usage
Run the emulator by passing the path to a Chip-8 ROM file as a command-line argument:
```
./chip8 <path_to_rom>
```
### CONTROLS.md
The emulator maps the Chip-8 keypad to the following keyboard keys:
```
| Chip-8 Key | Keyboard Key |
|------------|--------------|
| 1          | 1            |
| 2          | 2            |
| 3          | 3            |
| C          | 4            |
| 4          | Q            |
| 5          | W            |
| 6          | E            |
| D          | R            |
| 7          | A            |
| 8          | S            |
| 9          | D            |
| E          | F            |
| A          | Z            |
| 0          | X            |
| B          | C            |
| F          | V            |
-----------------------------
```
# Emulation Timing
- The Chip-8 timers run at 60 Hz.
- This emulator uses `SDL_Delay(17)` (approximately 17 milliseconds) in the main loop to approximate a 60 Hz refresh rate.
- Multiple CPU cycles can run per frame while timers update accordingly.

# Known Issues & Future Improvements
- **Undefined Opcodes:** Some opcodes might still be unimplemented or misinterpreted, triggering debug messages.
- **Input Handling:** Further refinement may be needed to capture key presses and releases accurately.
- **Sound:** Currently, sound is represented as a "BEEP!" message; actual audio output is not implemented.
- **Timing Accuracy:** The CPU cycle rate and timer updates could be decoupled for more precise emulation.
