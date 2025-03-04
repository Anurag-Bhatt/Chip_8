#include <iostream>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <time.h>

#include <SDL2/SDL.h>

#include "chip.h"

Chip_8::Chip_8()
{     
    unsigned char fontset[80] =
    {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
    };


    
    // Sets the pointers to their default values
    this->pc = 0x200;
    this->opcodes = 0;
    this->I = 0;
    this->sp = 0;
    
    this->drawFlag  = false;
    this->delay_timer = 0;
    this->sound_timer = 0;
    
    for(int i = 0; i < 16; i++){
        this->stack[i] = 0;
        this->key[i] = 0;
        this->V[i] = 0;
    }
    
    this->drawFlag = false;
    this->emulateFlag = false;

    for(int i = 0; i < 4096; i++){
        this->memory[i] = 0;
    }
    for(int i = 0; i < 2048; i++){
        this->gfx[i] = 0;
    }
    for(int i = 0; i < 80; i++){
        this->memory[i] = fontset[i];
    }

    srand(time(NULL));
}

Chip_8::~Chip_8()
{

}

bool Chip_8::LoadRom(const char *romPath)
{
    std::fstream romFile;
    romFile.open(romPath,std::ios::in | std::ios::binary);

    if(!romFile.is_open()){
        std::cerr << "Could not load rom at path: " << romPath << "\nCheck Path.\n";
        return false;
    }

    std::cout << "Reading data from ROM....\n";
    auto file_size = std::filesystem::file_size(romPath);
    std::cout << "ROM File size is: " << file_size << "\n";

    if(file_size > (4096-512)){
        std::cerr << "Could not Load ROM ERR: sizeof(ROM) > internal memory\n";
        
        return false;
    }

    // TODO: Make this into more C++ style than C
    char *bufferData = new char[file_size];
    if(bufferData == NULL){
        std::cerr << "ERROR: could not read rom\nBuffer not allocated\n";
    }
    romFile.read(bufferData, file_size);


    for(int i = 0; i < file_size; i++){
        this->memory[512 + i] = bufferData[i];
    }

    // PrintMemory();

    romFile.close();
    delete[] bufferData;
    std::cout << "Rom read successfully.\n";
    return true;

}

void Chip_8::EmulateCycle()
{
    
    // How do I emulate the cycles? By checking what the next opcode is.
    // Crazy brother GENUIUS!
    drawFlag = false;
    opcodes = ((memory[pc] << 8) | memory[pc+1]); 

    switch (opcodes & 0xF000)
    {
    case 0x0000:
            switch (opcodes & 0x00FF)
            {
            case 0x00E:
                // Clear the screen;
                for(int i = 0; i < 2048; i++){
                    gfx[i] = 0x0; 
                }
                drawFlag = true;
                pc += 2;
                break;
            case 0x0EE:
                // return? 
                sp--;
                pc = stack[sp];
                pc += 2;
                break;
            
            default:
                std::cout << "Undefined opcode: " << opcodes <<"\n";
                std::cout << "Opcode: 0x" << std::dec << opcodes << std::dec << "\n";
                pc += 2;
                break;
            }
        break;
    case 0x1000:
        pc = opcodes & 0x0FFF;
        break;
    case 0x2000:
        stack[sp] = pc;
        sp++;
        pc = opcodes & 0x0FFF;
        break;
    case 0x3000:
        if(V[(opcodes & 0x0F00) >> 8] == (opcodes&0x00FF)){
            pc += 4;
        }else{
            pc += 2;
        }
        break;
    case 0x4000:
        if(V[(opcodes & 0x0F00) >> 8] != (opcodes&0x00FF)){
            pc += 4;
        }else{
            pc += 2;
        }
        break;
    case 0x5000:
        if(V[(opcodes & 0x0F00) >> 8] == V[(opcodes & 0x0F0) >> 4]){
            pc += 4;
        }else{
            pc += 2;
        }
        break;
    case 0x6000:
        V[(opcodes & 0x0F00) >> 8] = (opcodes&0x00FF);
        pc += 2;
         
        break;
    case 0x7000:
        V[(opcodes & 0x0F00) >> 8] += (opcodes&0x00FF);
        pc += 2;

        break;
    case 0x8000:
       switch (opcodes & 0x000F)
       {
        case 0x0000:
            V[(opcodes & 0x0F00) >> 8] = V[(opcodes & 0x00F0) >> 4];
            pc += 2;
        break;
        case 0x0001:
            V[(opcodes & 0x0F00) >> 8] |= V[(opcodes & 0x00F0) >> 4];
            pc += 2;
        break;
        case 0X0002:
            V[(opcodes & 0x0F00) >> 8] &= V[(opcodes & 0x00F0) >> 4];
            pc += 2;
        break;
        case 0X0003:
            V[(opcodes & 0x0F00) >> 8] ^= V[(opcodes & 0x00F0) >> 4];
            pc += 2;
        break;
        case 0x0004:
            V[(opcodes & 0x0F00) >> 8] += V[(opcodes & 0x00F0) >> 4];
            pc += 2;
        break;
        case 0x0005:
            V[(opcodes & 0x0F00) >> 8] -= V[(opcodes & 0x00F0) >> 4];
            pc += 2;
        break;
        case 0x0006:
            V[(opcodes & 0x0F00) >> 8] >>= 1;
            pc += 2;
        break;
        case 0x0007:
            V[(opcodes & 0x0F00) >> 8] = V[(opcodes & 0x00F0) >> 4] - V[(opcodes & 0x0F00) >> 8];
            pc += 2;
        break;
        case 0x0008:
            V[(opcodes & 0x0F00) >> 8] <<= 1;
            pc += 2;
        break;
       default:
       std::cout << "Undefined opcode: " << opcodes << "\n";
       std::cout << "Opcode: 0x" << std::dec << opcodes << std::dec << "\n";
       pc += 2;
        break;
       }
        break;
    case 0x9000:
        if(V[(opcodes & 0x0F00) >> 8] != V[(opcodes & 0x0F0) >> 4]){
            pc += 4;
        }else{
            pc += 2;
        }
        break;
    case 0xA000:
        this->I = (opcodes & 0xFFF);
        pc += 2;
        break;
    case 0xB000:
        this->pc = V[0] + (opcodes & 0xFFF);
        pc += 2;
        break;
    case 0xC000:
        // uses srand(time(NULL)) in the constructor when intiatating chip-8 object
        V[(opcodes & 0X0F00) >> 8] = rand() & ((opcodes & 0x00FF));
        pc += 2;
        break;
    case 0xD000:
        // Draws the sprite at (Vx,Vy) and height N -> 0xDXYN;
        {

            unsigned short x = V[(opcodes & 0x0F00) >> 8];
            unsigned short y = V[(opcodes & 0x00F0) >> 4];
            unsigned short height = opcodes & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for(int yline = 0; yline < height; yline++){
                pixel = this->memory[I + yline];
                for(int xline = 0; xline < 8; xline++){

                    if(pixel & (0x80 >> xline) != 0){
                        if(gfx[x + xline + ((y + yline) * 64)] == 1){
                            V[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }
            drawFlag = true;
            pc += 2;
        }
        break;
    case 0xE000:
        switch (opcodes & 0x000F)
        {
        case 0x000E:
        // Here too -- done
            if(key[V[(opcodes & 0x0F00) >> 8]] != 0)
            {
                pc += 4;
            }else{
                pc += 2;
            }
        break;
        case 0x0001:
        // Implement -- done
            if(key[V[(opcodes & 0x0F00) >> 8]] == 0)
            {
                pc += 4;
            }else{
                pc += 2;
            }
        break;
        default:
            std::cout << "Undefined opcode: " << opcodes << "\n";
            std::cout << "Opcode: 0x" << std::dec << opcodes << std::dec << "\n";
            pc += 2;
            break;
        }
        break;
    case 0xF000:
        switch (opcodes & 0x000F)
        {
        case 0x0007:
            V[(opcodes & 0x0F00) >> 8] = delay_timer; // Implement
            pc += 2;
        break;
        case 0x000A:    
            // Implement -- done, awaits a keypress and stores it in Vx;
            {
                bool keyPress = false;

                for(int i=0;i<16;i++){
                    if(key[i] != 0){
                        V[(opcodes & 0x0F00) >> 8] = key[i];
                        keyPress = true;
                    }
                }
                // if no key press then returns, skips the cycles
                if(keyPress == false){
                    return;
                }
                pc += 2;
            }
        break;
        case 0x0005:
            switch (opcodes & 0x00F0)
            {
            case 0x0010:
            delay_timer = V[(opcodes & 0x0F00) >> 8];
                pc += 2;
            break;
            case 0x0050:
                {
                // This functions need the array V the position of I and the index of the 
                // end point ie point till it should read into the mem buffer.
                int x = V[(opcodes & 0x0F00) >> 8];
                for(int i=0;i<x;i++){
                    memory[I + i] = V[i];
                }
                I += x + 1;
                pc += 2;
            }
            break;
            case 0x0060:
            {
                int x = V[(opcodes & 0x0F00) >> 8];
                for(int i=0;i<x;i++){
                    V[i] = memory[I + i];
                }
                I += x + 1;
                pc += 2;
            }
            break;
            default:
                std::cout << "Undefined opcodes: " << opcodes << "\n";
                std::cout << "Opcode: 0x" << std::dec << opcodes << std::dec << "\n";
                pc += 2;
            break;
            }
        break;
        case 0x0008:
            // Implement -- done
            sound_timer = V[(opcodes & 0x0F00) >> 8];
            pc += 2;
        break;
        case 0x000E:
            this->I += V[(opcodes & 0x0F00) >> 8];
            pc += 2;
        break;
        case 0x0009:
            // Icouldnt figure this out, looked at a guide's solution.
            this->I += V[(opcodes & 0x0F00) >> 8] * 0x5;
            pc += 2;
        break;
        case 0x0003:{
            // Sets BCD(V[x]) to I, I+1, I+2
            this->memory[I] = V[(opcodes & 0x0F00) >> 8] / 100;
            this->memory[I+1] = (V[(opcodes & 0x0F00) >> 8] / 10) % 10;
            this->memory[I+2] = V[(opcodes & 0x0F00) >> 8] % 10;
            pc += 2;
            }
        break;
        default:
            std::cout << "Undefined opcodes: " << opcodes << "\n";
            std::cout << "Opcode: 0x" << std::dec << opcodes << std::dec << "\n";
            pc += 2;
            break;
        }
        break;
    default: 
        std::cout << "Undefined opcodes: " << opcodes << "\n"; 
        std::cout << "Opcode: 0x" << std::dec << opcodes << std::dec << "\n";
        pc += 2;
    break;
    }

    if(delay_timer > 0){
        --delay_timer;
    }

    if(sound_timer > 0){
        
        if(sound_timer == 1){
            std::cout << "BEEP!\n";
        }
        --sound_timer;
    }

}

bool Chip_8::ShouldEmulate()
{
    // Until the sdl input is an escape key this should set emulate flag to true
    emulateFlag = true;
    return emulateFlag;
}

bool Chip_8::ShouldDraw()
{
    // when chip 8 reads or decides its time to draw, this should set(idk how) draw flag to true
    // otherwise this should be set to false at the begining of every cycle.
    return drawFlag;
}


void Chip_8::CheckInput()
{
    
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    // Clear previous key state (set all 16 keys to 0)
    for (int i = 0; i < 16; i++) {
        key[i] = 0;
    }

    // Map SDL scancodes to Chip‑8 keys
    key[0] = keystate[SDL_SCANCODE_1];
    key[1] = keystate[SDL_SCANCODE_2];
    key[2] = keystate[SDL_SCANCODE_3];
    key[3] = keystate[SDL_SCANCODE_4];

    key[4] = keystate[SDL_SCANCODE_Q];
    key[5] = keystate[SDL_SCANCODE_W];
    key[6] = keystate[SDL_SCANCODE_E];
    key[7] = keystate[SDL_SCANCODE_R];

    key[8] = keystate[SDL_SCANCODE_A];
    key[9] = keystate[SDL_SCANCODE_S];
    key[10] = keystate[SDL_SCANCODE_D];
    key[11] = keystate[SDL_SCANCODE_F];

    key[12] = keystate[SDL_SCANCODE_Z];
    key[13] = keystate[SDL_SCANCODE_X];
    key[14] = keystate[SDL_SCANCODE_C];
    key[15] = keystate[SDL_SCANCODE_V];
}

void Chip_8::PrintMemory()
{
    std::cout << "Printing out the Memory\n";
    for(int i = 0;i < sizeof(memory); i++){
        std::cout << memory[512 + i] << " ";
        if(i == 20) std::cout << "\n";
    }
}

void Chip_8::DebugDraw()
{
    
    for(int y = 0;y < 32; y++){
        for(int x = 0; x < 64; x++){

            if(gfx[(y*64) + x] == 0){
                std::cout << "0";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

}

const unsigned char *Chip_8::Getgfx()
{
    return gfx;
}

const unsigned char *Chip_8::GetMem()
{
    return memory;
}
