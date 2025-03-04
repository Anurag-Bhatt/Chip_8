#include <iostream>
#include <SDL2/SDL.h>
#include "chip.h" // Your Chip-8 header

// Define window and Chip-8 display dimensions
const int CHIP8_WIDTH = 64;
const int CHIP8_HEIGHT = 32;
const int WINDOW_WIDTH = 640;   // For example, scaling 64x32 to 640x320
const int WINDOW_HEIGHT = 320;

int main(int argc, char **argv) {
    std::cout << "Emulating Chip-8\n";

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
        return -1;
    }
    
    // Create a window
    SDL_Window* window = SDL_CreateWindow("Chip-8 Emulator",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }
    
    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Create an SDL texture for the Chip-8 display (64 x 32 pixels)
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             CHIP8_WIDTH,
                                             CHIP8_HEIGHT);
    if (!texture) {
        std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Create and initialize the Chip-8 emulator
    Chip_8 chip;
    std::string romFilePath = "NO_ROM";
    if (argc > 1) {
        romFilePath = argv[1];
        if (!chip.LoadRom(romFilePath.c_str())) {
            SDL_DestroyTexture(texture);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -1;
        }
    } else {
        std::cout << "No ROM supplied, initiating default behaviour.\n";
        std::cout << "Usage: chip_8 <ROMFILEPATH>\n";
    }
    
    bool quit = false;
    SDL_Event event;


    // Main emulation loop
    while (chip.ShouldEmulate() && !quit) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = true;
        }
        
        chip.EmulateCycle();
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        if (chip.ShouldDraw()) {
            // Get the graphics buffer from Chip-8.
            const unsigned char* gfxBuffer = chip.Getgfx();
            
            // Convert each pixel: 0->black, 1->white
            uint32_t pixels[CHIP8_WIDTH * CHIP8_HEIGHT];
            for (int i = 0; i < CHIP8_WIDTH * CHIP8_HEIGHT; i++) {
                pixels[i] = (gfxBuffer[i] ? 0xFFFFFFFF : 0x000000FF);
            }
            
            // Update the texture with new pixel data
            SDL_UpdateTexture(texture, nullptr, pixels, CHIP8_WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
        
        chip.CheckInput();
        
        SDL_Delay(17); // Chip8 runs at 60hz, which is ~= 17 ms;
    }
    
    // Clean up SDL resources
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
