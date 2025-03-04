#ifndef CHIP_H
#define CHIP_H



class Chip_8{

    // The constructor initalize the chip ie sets everything to 0
public:
    Chip_8();
    ~Chip_8();

    bool LoadRom(const char* romPath);
    void EmulateCycle();

    bool ShouldEmulate();
    bool ShouldDraw();

    void CheckInput();

    void PrintMemory();

    void DebugDraw();

    const unsigned char *Getgfx();
    const unsigned char *GetMem();

private:

  // What all should be private?
    unsigned short opcodes;
    unsigned short I;
    unsigned short pc;
    unsigned short sp;

    unsigned char delay_timer, sound_timer;

    unsigned short V[16];
    unsigned char key[16];
    unsigned short stack[16];
    unsigned char memory[4096];
    unsigned char gfx[64*32];

    bool drawFlag, emulateFlag;

};

#endif