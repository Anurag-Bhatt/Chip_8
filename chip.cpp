#include <iostream>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <time.h>

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

    // Loading font into the memory
    // for(int i =0;i<80;i++){
    //     this->memory[i] = fontset[i];
    // }

    memset(this->memory, 0, sizeof(this->memory));
    memset(this->gfx, 0, sizeof(this->gfx));
    memcpy(memory,fontset,80);

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
    romFile.read(bufferData, file_size);


    // upper limit, TOOD: figure out this
    for(int i = 0; i < file_size; i++){
        this->memory[512 + i] = bufferData[i];
    }

    // PrintMemory();

    delete[] bufferData;
    std::cout << "Rom read successfully.\n";
    return true;

}

void Chip_8::EmulateCycle()
{
    srand(time(NULL));
    std::cout <<"Emulating next cycle of Chip\n";

    // How do I emulate the cycles? By checking what the next opcode is.
    // Crazy brother GENUIUS!

    opcodes = ((memory[pc] << 8) | memory[pc+1]); 

    switch (opcodes & 0xF000)
    {
    case 0x0000:
            switch (opcodes & 0x00FF)
            {
            case 0x00E:
                // Clear the screen;
                for(int i =0;i<2048;i++){
                    gfx[i] = 0x0;
                    drawFlag = true;
                    pc += 2;
                }
                break;
            case 0x0EE:
                // return? 
                sp--;
                pc = stack[sp];
                pc += 2;
                break;
            
            default:
                std::cout << "Undefined opcode: " << opcodes <<"\n";
                break;
            }
        break;
    case 0x1000:
        pc = opcodes & 0x0FFF;
        pc += 2;
        break;
    case 0x2000:
        stack[sp] = pc;
        pc = opcodes & 0x0FFF;
        pc += 2;
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
        this->I = (opcodes & 0xFFF) >> 4;
        pc += 2;
        break;
    case 0xB000:
        this->pc = V[0] + (opcodes & 0xFFF) >> 4;
        pc += 2;
        break;
    case 0xC000:
        // Uses srand(time(NULL)) in begining of every chip.EmulateCycle() call... 
        // I assume this would move the seed every time this function is called
        // and if the function reaches here and calls rand() it would return same 'random'
        // number.. I hope I am right on this assumtion.
        V[(opcodes & 0X0F00) >> 8] = rand() & ((opcodes & 0x00FF));
        pc += 2;
        break;
    case 0xD000:
        // Draw(x,y,n);
        // Draws the sprite at (Vx,Vy) and height N -> 0xDXYN;
        Draw(V[(opcodes & 0x0F00) >> 8],V[(opcodes & 0x00F0) >> 4],V[(opcodes & 0x000F)]);
        pc += 2;
        break;
    case 0xE000:
        switch (opcodes & 0x000F)
        {
        case 0x000E:
        // Here too
            if(Key() == V[(opcodes & 0x0F00) >> 8])
            {
                pc += 4;
            }else{
                pc += 2;
            }
        break;
        case 0x0001:
        // Implement
            if(Key() != V[(opcodes & 0x0F00) >> 8])
            {
                pc += 4;
            }else{
                pc += 2;
            }
        break;
        default:
            std::cout << "Undefined opcode: " << opcodes << "\n";
            pc += 2;
            break;
        }
        break;
    case 0xF000:
        switch (opcodes & 0x000F)
        {
        case 0x0007:
            V[(opcodes & 0x0F00) >> 8] = GetDelay(); // Implement
            pc += 2;
        break;
        case 0x000A:    
            // Implement
            V[(opcodes & 0x0F00) >> 8] = GetKey();
            pc += 2;
        break;
        case 0x0005:
            switch (opcodes & 0x00F0)
            {
            case 0x0010:
                SetDelay(V[(opcodes & 0x0F00) >> 8]);
                pc += 2;
            break;
            case 0x0050:
            // Implement RegDump and RegLoad functions
                // This functions need the array V the position of I and the index of the 
                // end point ie point till it should read into the mem buffer.
                int x = V[(opcodes & 0x0F00) >> 8];
                RegDump(V,I,x);
                pc += 2;
            break;
            case 0x0060:
                int x = V[(opcodes & 0x0F00) >> 8];
                RegLoad(V,I,x);
                pc += 2;
            break;
            default:
                std::cout << "Undefined opcodes: " << opcodes << "\n";
                pc += 2;
            break;
            }
        break;
        case 0x0008:
            // Implement 
            SetSound(V[(opcodes & 0x0F00) >> 8]);
            pc += 2;
        break;
        case 0x000E:
            this->I += V[(opcodes & 0x0F00) >> 8];
            pc += 2;
        break;
        case 0x0009:
            // Implement
            this->I += SetSprite(V[(opcodes & 0x0F00) >> 8]);
            pc += 2;
        break;
        case 0x0003:
            // Sets BCD to I, I+1, I+2
            // TODO: Figure out
            auto value = V[(opcodes & 0x0F00) >> 8];
            this->memory[I] = value / 100;
            this->memory[I] = value / 10;
            this->memory[I] = value % 10;
            pc += 2;
        break;
        default:
            std::cout << "Undefined opcodes: " << opcodes << "\n";
            pc += 2;
            break;
        }
        break;
    default: 
        std::cout << "Undefined opcodes: " << opcodes << "\n"; 
        pc += 2;
    break;
    }

}

bool Chip_8::ShouldEmulate()
{
    return emulateFlag;
}

bool Chip_8::ShouldDraw()
{
    return drawFlag;
}

void Chip_8::DrawGraphics()
{
    std::cout <<"Drawing Graphics of this Chip\n";

}

void Chip_8::CheckInput()
{
    std::cout <<"Checking input for this cycle of Chip\n";
}

void Chip_8::PrintMemory()
{
    std::cout << "Printing out the Memory\n";
    for(int i = 0;i < sizeof(memory); i++){
        std::cout << memory[512 + i] << " ";
        if(i == 20) std::cout << "\n";
    }
}

void Chip_8::Draw(unsigned char x, unsigned char y, unsigned char N)
{
    // Draw the sprite at (x,y) with constant row width of 8 btyes and N rows high
}


void Chip_8::RegDump(unsigned short *V, unsigned short I, int x)
{
    // copies V[i] into mem[I+i] till X

    for(int i=0;i<x;i++){
        this->memory[I + i] = V[i];
    }
}

void Chip_8::RegLoad(unsigned short *V, unsigned short I, int x)
{
    // copies V[i] into mem[I+i] till X

    for(int i=0;i<x;i++){
        V[i] = this->memory[I + i];
    }
}

