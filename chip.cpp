#include <iostream>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <ctime> 
#include <SDL2/SDL.h>

#include "chip.h"

Chip_8::Chip_8() {
    // Chip‑8 fontset (80 bytes)
    unsigned char fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    // Initialize registers and memory
    pc = 0x200;       // Program counter starts at 0x200
    opcodes = 0;
    I = 0;
    sp = 0;

    delay_timer = 0;
    sound_timer = 0;
    drawFlag = false;
    emulateFlag = true;

    // Clear registers, stack, and keys
    for (int i = 0; i < 16; i++) {
        V[i] = 0;
        stack[i] = 0;
        key[i] = 0;
    }
    // Clear memory and gfx
    memset(memory, 0, sizeof(memory));
    memset(gfx, 0, sizeof(gfx));

    // Load fontset into memory
    for (int i = 0; i < 80; i++) {
        memory[i] = fontset[i];
    }

    // Seed random number generator
    srand(static_cast<unsigned int>(time(nullptr)));
}

Chip_8::~Chip_8() {
    
}

bool Chip_8::LoadRom(const char *romPath) {
    std::fstream romFile;
    romFile.open(romPath, std::ios::in | std::ios::binary);

    if (!romFile.is_open()) {
        std::cerr << "Could not load ROM at path: " << romPath << "\nCheck Path.\n";
        return false;
    }

    std::cout << "Reading data from ROM....\n";
    auto file_size = std::filesystem::file_size(romPath);
    std::cout << "ROM File size is: " << file_size << "\n";

    if (file_size > (4096 - 512)) {
        std::cerr << "Could not Load ROM ERR: ROM size exceeds available memory\n";
        return false;
    }

    char *bufferData = new char[file_size];
    romFile.read(bufferData, file_size);

    // Load ROM into memory starting at 0x200
    for (size_t i = 0; i < file_size; i++) {
        memory[512 + i] = static_cast<unsigned char>(bufferData[i]);
    }

    romFile.close();
    delete[] bufferData;
    std::cout << "ROM read successfully.\n";
    return true;
}

void Chip_8::EmulateCycle() {
    std::cout << "Emulating next cycle of Chip\n";

    // Fetch opcode (two bytes)
    opcodes = (memory[pc] << 8) | memory[pc + 1];

    // For debugging
    std::cout << "Opcode: 0x" << std::hex << opcodes << std::dec << "\n";

    drawFlag = false;

    // Decode opcode using the high nibble
    switch (opcodes & 0xF000) {
        case 0x0000:
            switch (opcodes & 0x00FF) {
                case 0x00E0:  // Clear the display
                    memset(gfx, 0, sizeof(gfx));
                    drawFlag = true;
                    pc += 2;
                    break;
                case 0x00EE:  // Return from subroutine
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;
                default:
                    std::cout << "Undefined opcode: 0x" << std::hex << opcodes << std::dec << "\n";
                    pc += 2;
                    break;
            }
            break;

        case 0x1000:  // Jump to address NNN
            pc = opcodes & 0x0FFF;
            break;

        case 0x2000:  // Call subroutine at NNN
            stack[sp] = pc;
            sp++;
            pc = opcodes & 0x0FFF;
            break;

        case 0x3000: { // Skip next instruction if Vx == kk
            uint8_t x = (opcodes & 0x0F00) >> 8;
            uint8_t kk = opcodes & 0x00FF;
            pc += (V[x] == kk) ? 4 : 2;
            break;
        }

        case 0x4000: { // Skip next instruction if Vx != kk
            uint8_t x = (opcodes & 0x0F00) >> 8;
            uint8_t kk = opcodes & 0x00FF;
            pc += (V[x] != kk) ? 4 : 2;
            break;
        }

        case 0x5000: { // Skip next instruction if Vx == Vy
            uint8_t x = (opcodes & 0x0F00) >> 8;
            uint8_t y = (opcodes & 0x00F0) >> 4;
            pc += (V[x] == V[y]) ? 4 : 2;
            break;
        }

        case 0x6000: { // Set Vx = kk
            uint8_t x = (opcodes & 0x0F00) >> 8;
            V[x] = opcodes & 0x00FF;
            pc += 2;
            break;
        }

        case 0x7000: { // Set Vx = Vx + kk
            uint8_t x = (opcodes & 0x0F00) >> 8;
            V[x] += opcodes & 0x00FF;
            pc += 2;
            break;
        }

        case 0x8000: {
            uint8_t x = (opcodes & 0x0F00) >> 8;
            uint8_t y = (opcodes & 0x00F0) >> 4;
            switch (opcodes & 0x000F) {
                case 0x0:  // Set Vx = Vy
                    V[x] = V[y];
                    pc += 2;
                    break;
                case 0x1:  // Set Vx = Vx OR Vy
                    V[x] |= V[y];
                    pc += 2;
                    break;
                case 0x2:  // Set Vx = Vx AND Vy
                    V[x] &= V[y];
                    pc += 2;
                    break;
                case 0x3:  // Set Vx = Vx XOR Vy
                    V[x] ^= V[y];
                    pc += 2;
                    break;
                case 0x4: { // Set Vx = Vx + Vy, set VF = carry
                    uint16_t sum = V[x] + V[y];
                    V[0xF] = (sum > 255) ? 1 : 0;
                    V[x] = sum & 0xFF;
                    pc += 2;
                    break;
                }
                case 0x5: { // Set Vx = Vx - Vy, set VF = NOT borrow
                    V[0xF] = (V[x] > V[y]) ? 1 : 0;
                    V[x] -= V[y];
                    pc += 2;
                    break;
                }
                case 0x6: { // Set Vx = Vx >> 1; VF = least significant bit before shift
                    V[0xF] = V[x] & 0x1;
                    V[x] >>= 1;
                    pc += 2;
                    break;
                }
                case 0x7: { // Set Vx = Vy - Vx, set VF = NOT borrow
                    V[0xF] = (V[y] > V[x]) ? 1 : 0;
                    V[x] = V[y] - V[x];
                    pc += 2;
                    break;
                }
                case 0xE: { // Set Vx = Vx << 1; VF = most significant bit before shift
                    V[0xF] = (V[x] & 0x80) >> 7;
                    V[x] <<= 1;
                    pc += 2;
                    break;
                }
                default:
                    std::cout << "Undefined opcode in 0x8000 group: 0x" << std::hex << opcodes << std::dec << "\n";
                    pc += 2;
                    break;
            }
            break;
        }

        case 0x9000: { // Skip next instruction if Vx != Vy
            uint8_t x = (opcodes & 0x0F00) >> 8;
            uint8_t y = (opcodes & 0x00F0) >> 4;
            pc += (V[x] != V[y]) ? 4 : 2;
            break;
        }

        case 0xA000: // Set I = NNN
            I = opcodes & 0x0FFF;
            pc += 2;
            break;

        case 0xB000: // Jump to address V0 + NNN
            pc = V[0] + (opcodes & 0x0FFF);
            break;

        case 0xC000: { // Set Vx = random byte AND kk
            uint8_t x = (opcodes & 0x0F00) >> 8;
            V[x] = (rand() % 256) & (opcodes & 0x00FF);
            pc += 2;
            break;
        }

        case 0xD000: { // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            uint8_t x = V[(opcodes & 0x0F00) >> 8];
            uint8_t y = V[(opcodes & 0x00F0) >> 4];
            uint8_t height = opcodes & 0x000F;  // Use lower nibble as height
            uint8_t pixel;
            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    // Correct operator precedence with parentheses
                    if ((pixel & (0x80 >> xline)) != 0) {
                        int index = x + xline + ((y + yline) * 64);
                        // Check screen boundaries (optional)
                        if (index < (64 * 32)) {
                            if (gfx[index] == 1)
                                V[0xF] = 1;
                            gfx[index] ^= 1;
                        }
                    }
                }
            }
            drawFlag = true;
            pc += 2;
            break;
        }

        case 0xE000: {
            uint8_t x = (opcodes & 0x0F00) >> 8;
            switch (opcodes & 0x00FF) {
                case 0x9E:  // Skip next instruction if key with the value of Vx is pressed
                    pc += (key[V[x]] != 0) ? 4 : 2;
                    break;
                case 0xA1:  // Skip next instruction if key with the value of Vx is not pressed
                    pc += (key[V[x]] == 0) ? 4 : 2;
                    break;
                default:
                    std::cout << "Undefined opcode in 0xE000 group: 0x" << std::hex << opcodes << std::dec << "\n";
                    pc += 2;
                    break;
            }
            break;
        }

        case 0xF000: {
            uint8_t x = (opcodes & 0x0F00) >> 8;
            switch (opcodes & 0x00FF) {
                case 0x07:  // Set Vx = delay timer value
                    V[x] = delay_timer;
                    pc += 2;
                    break;
                case 0x0A: { // Wait for a key press, store the value of the key in Vx.
                    bool keyPress = false;
                    for (int i = 0; i < 16; i++) {
                        if (key[i] != 0) {
                            V[x] = i;
                            keyPress = true;
                            break;
                        }
                    }
                    if (!keyPress) {
                        return; // Skip cycle if no key press detected.
                    }
                    pc += 2;
                    break;
                }
                case 0x15:  // Set delay timer = Vx
                    delay_timer = V[x];
                    pc += 2;
                    break;
                case 0x18:  // Set sound timer = Vx
                    sound_timer = V[x];
                    pc += 2;
                    break;
                case 0x1E:  // Set I = I + Vx
                    I += V[x];
                    pc += 2;
                    break;
                case 0x29:  // Set I = location of sprite for digit Vx
                    // Typically, each sprite is 5 bytes long and stored at beginning of memory.
                    I = V[x] * 5;
                    pc += 2;
                    break;
                case 0x33: { // Store BCD representation of Vx in memory locations I, I+1, and I+2.
                    uint8_t value = V[x];
                    memory[I]   = value / 100;
                    memory[I+1] = (value / 10) % 10;
                    memory[I+2] = value % 10;
                    pc += 2;
                    break;
                }
                case 0x55: { // Store registers V0 through Vx in memory starting at address I.
                    for (int i = 0; i <= x; i++) {
                        memory[I + i] = V[i];
                    }
                    // Some interpreters do not modify I; others do: adjust as needed.
                    pc += 2;
                    break;
                }
                case 0x65: { // Read registers V0 through Vx from memory starting at address I.
                    for (int i = 0; i <= x; i++) {
                        V[i] = memory[I + i];
                    }
                    // Some interpreters do not modify I; adjust as needed.
                    pc += 2;
                    break;
                }
                default:
                    std::cout << "Undefined opcode in 0xF000 group: 0x" << std::hex << opcodes << std::dec << "\n";
                    pc += 2;
                    break;
            }
            break;
        }

        default:
            std::cout << "Undefined opcode: 0x" << std::hex << opcodes << std::dec << "\n";
            pc += 2;
            break;
    }

    // Update timers
    if (delay_timer > 0)
        delay_timer--;

    if (sound_timer > 0) {
        if (sound_timer == 1)
            std::cout << "BEEP!\n";
        sound_timer--;
    }
}

bool Chip_8::ShouldEmulate() {
    // Return true as long as emulation should continue.
    return emulateFlag;
}

bool Chip_8::ShouldDraw() {
    return drawFlag;
}

void Chip_8::CheckInput() {
    std::cout << "Checking input for this cycle of Chip\n";
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    // Clear previous key state
    for (int i = 0; i < 16; i++) {
        key[i] = 0;
    }

    // Map specific SDL scancodes to Chip‑8 keys
    key[0x1] = keystate[SDL_SCANCODE_1];
    key[0x2] = keystate[SDL_SCANCODE_2];
    key[0x3] = keystate[SDL_SCANCODE_3];
    key[0xC] = keystate[SDL_SCANCODE_4];

    key[0x4] = keystate[SDL_SCANCODE_Q];
    key[0x5] = keystate[SDL_SCANCODE_W];
    key[0x6] = keystate[SDL_SCANCODE_E];
    key[0xD] = keystate[SDL_SCANCODE_R];

    key[0x7] = keystate[SDL_SCANCODE_A];
    key[0x8] = keystate[SDL_SCANCODE_S];
    key[0x9] = keystate[SDL_SCANCODE_D];
    key[0xE] = keystate[SDL_SCANCODE_F];

    key[0xA] = keystate[SDL_SCANCODE_Z];
    key[0x0] = keystate[SDL_SCANCODE_X];
    key[0xB] = keystate[SDL_SCANCODE_C];
    key[0xF] = keystate[SDL_SCANCODE_V];
}

void Chip_8::PrintMemory() {
    std::cout << "Printing out the Memory\n";
    // Print a portion of memory for debugging, starting at 0x200
    for (int i = 0; i < 64; i++) {
        std::cout << static_cast<int>(memory[512 + i]) << " ";
        if ((i+1) % 16 == 0)
            std::cout << "\n";
    }
}

void Chip_8::DebugDraw() {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            std::cout << (gfx[y * 64 + x] ? "1" : "0");
        }
        std::cout << "\n";
    }
}

const unsigned char *Chip_8::Getgfx() {
    return gfx;
}

const unsigned char *Chip_8::GetMem()
{
    return memory;
}
