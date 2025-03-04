default: chip8

chip8:
	g++ -std=c++17 -o chip8 Application.cpp chip.cpp $(shell sdl2-config --cflags --libs)

run: chip8
	./chip8 "No_ROM_Provided"

clean:
	rm chip8
