#pragma once

#include <cstdint>
#include <array>
#include <chrono>
#include <random>

/* A chip-8 emulator.

System memory map:

0x000-0x1FF: Chip 8 interpreter (in this case only the fonts)
0x050-0x0A0: Reserved for the font
0x200-0xFFF: Program ROM and work RAM
*/

class Emulator
{
public:
	Emulator();
	~Emulator();

	//Resets all internal members
	void Reset();

	//Loads the desired ROM inside the memory
	void LoadRom(const std::string FilePath);

	//Compute one emulation cycle
	void ExecuteCycle();

	//Stores the state of the hex keyboard
	uint8_t Keymap[16] {};

	//Stores the state of each individual pixel of the 64x32 screen
	uint32_t ScreenPixels[64 * 32]{};

	std::string GetRomName() const;

private:
	//The ROM being played
	std::string RomName;

	//The CPU registers, last one is reserved as flag
	uint8_t Registers[16] {};

	//4096 bytes of memory for application
	uint8_t Memory[4096] {};

	//The index register to reference memory locations
	uint16_t I;

	//The program counter that stores the current instruction
	uint16_t PC;

	//The stack that stores PC address when executing subroutines
	uint16_t Stack[16] {};

	//The stack pointer that keeps track of elements in the stack
	uint8_t SP;

	//60 Hz clock for game timing
	uint8_t DelayTimer;

	//60 Hz clock for sound effects
	uint8_t SoundTimer;

	//Current opcode
	uint16_t Opcode;


	//Random Number Generator
	std::default_random_engine randGen;
	std::uniform_int_distribution <unsigned short> randByte;
};

