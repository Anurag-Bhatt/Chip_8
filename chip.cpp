#include <iostream>
#include <cstring>
#include <fstream>

#include "chip.h"

Chip_8::Chip_8()
{
    
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

    // TODO: Fill memory with font and required data before running
    memset(this->memory, 0, sizeof(this->memory));
    memset(this->gfx, 0, sizeof(this->gfx));

}

bool Chip_8::LoadRom(const char *romPath)
{
    std::fstream romFile;
    romFile.open(romPath,std::ios::binary);

    if(!romFile){
        std::cerr << "Rom Could not be opened\n";
        return false;
    }

    unsigned char bufferData[4096];
    int buffersize; // upper limit, TOOD: figure out this
    for(int i = 0;i <buffersize; i++){
        this->memory[512 + i ] = bufferData[i];
    }

    std::cout << "Rom read successfully.\n";
    return true;

}

void Chip_8::EmulateCycle()
{
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
}

void Chip_8::CheckInput()
{
}
