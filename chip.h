#ifndef CHIP_H
#define CHIP_H



class Chip_8{

    // The constructor initalize the chip ie sets everything to 0
public:
    Chip_8();

    bool LoadRom(const char* romPath);
    void EmulateCycle();

    bool ShouldEmulate();
    bool ShouldDraw();

    void DrawGraphics();
    void CheckInput();

    void PrintMemory();

private:

    void Draw(unsigned char x, unsigned char y, unsigned char N);
    unsigned char Key();
    unsigned char GetDelay();
    unsigned char SetDelay(unsigned short v);
    unsigned char SetSound(unsigned short v);
    unsigned char GetKey();

    unsigned short SetSprite(unsigned short v);

    // void RegDump(unsigned short v, unsigned short &I);
    void RegDump(unsigned short *, unsigned short , int);
    void RegLoad(unsigned short *, unsigned short , int);

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