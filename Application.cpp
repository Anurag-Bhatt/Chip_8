#include <iostream>

#include "chip.h"

// TODO: Figure out how to display the graphics using SDL.
// TODO: Implement the CH-8 functionality.


int main(int argc, char **argv){

    // Initializing the program
    std::cout << "Emulating Chip-8\n";
    
    Chip_8 chip;
    std::string romFilePath = "NO_ROM";

    if(argc > 1){
        romFilePath = argv[1];
        if(!chip.LoadRom(romFilePath.c_str())){
            return -1;
        }
    }
    else{
        std::cout << "No ROM supplied, initiating default behaviour.\n";
        std::cout << "Usage: chip_8 <ROMFILEPATH> \n";
    }

    // Init chip_8 -> done by the constructor

    while(chip.ShouldEmulate()){
        // Emulate one cycle of chip

        chip.EmulateCycle();

        // Draw the graphics

        if(chip.ShouldDraw()){
            chip.DrawGraphics();
        }

        // Checks for input

        chip.CheckInput();

        // Continue the loop till the chip reads instructions
    }


    return 0;
}