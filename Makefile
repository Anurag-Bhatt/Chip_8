default: chip_8

chip_8:
	g++ -o chip_8 Application.cpp

run:
	./chip_8 "No_ROM_Provided"

clean:
	rm chip_8