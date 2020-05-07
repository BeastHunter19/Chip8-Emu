#include "Emulator.h"
#include <fstream>
#include <iostream>

//The first available memory address for loading the rom data
static constexpr uint16_t START_ADDRESS = 0x200;

//The starting memory location for the font
static constexpr uint16_t FONT_START_ADDRESS = 0x50;

//Number of sprites in the font
static constexpr uint16_t FONTSET_SIZE = 80;

//The fontset for chip8
std::array<uint8_t, FONTSET_SIZE> Fontset =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Emulator::Emulator(): randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	//Load font into memory
	for (uint16_t i = 0; i < FONTSET_SIZE; i++)
	{
		Memory[FONT_START_ADDRESS + i] = Fontset[i];
	}

	//Initialize RNG
	randByte = std::uniform_int_distribution <unsigned short> (0, 255U);
	
	Reset();
}

Emulator::~Emulator()
{
}


void Emulator::Reset()
{
	//Reset everything
	PC = START_ADDRESS;
	Opcode = 0;
	I = 0;
	SP = 0;

	for (int V = 0; V < 16; V++)
	{
		Registers[V] = 0;
	}

	for (int Byte = 0; Byte < 4096; Byte++)
	{
		Memory[Byte] = 0;
	}

	for (int S = 0; S < 16; S++)
	{
		Stack[S] = 0;
	}

	for (int Key = 0; Key < 16; Key++)
	{
		Keymap[Key] = 0;
	}

	for (int Pixel = 0; Pixel < 2048; Pixel++)
	{
		ScreenPixels[Pixel] = 0;
	}

	DelayTimer = 0;
	SoundTimer = 0;

	RomName = " ";
}

void Emulator::LoadRom(const std::string FilePath)
{
	//Open the rom file as binary and point at the end (ate)
	std::ifstream rom(FilePath, std::ios::binary | std::ios::ate);

	if (rom.is_open())
	{
		//Get the size of the file and allocate an array as buffer
		std::streampos size = rom.tellg();
		char* buffer = new char[size];

		//Go back to the beginning and fill the buffer
		rom.seekg(0, std::ios::beg);
		rom.read(buffer, size);
		rom.close();

		//Load the ROM content into the Emulator memory
		for (long i = 0; i < size; i++)
		{
			Memory[START_ADDRESS + i] = buffer[i];
		}

		delete [] buffer;

		RomName = FilePath.substr(FilePath.find_last_of('\\'));
	}
}

void Emulator::ExecuteCycle()
{
	//Fetch opcode
	Opcode = Memory[PC] << 8 | Memory[PC + 1];

	//Debug only
	std::cout << std::hex << Opcode << "\n";
	for (int V = 0; V < 16; V++) {
		std::cout << "V" << V << ": " << std::hex << Registers[V] << "  ";
	}
	std::cout << "\n";

	//Increments PC
	PC += 2;

	//Decode opcode
	switch (Opcode & 0xF000)
	{
		case 0x0000:
		{
			switch (Opcode & 0x000F)
			{
				case 0x0000:	//00E0: Clears the screen
				{
					memset(ScreenPixels, 0, sizeof(ScreenPixels));
					break;
				}
				case 0x000E:	//00EE: Returns from a subroutine
				{
					--SP;
					PC = Stack[SP];
					break;
				}
			}
			break;
		}
		case 0x1000:	//1NNN: Jump to address NNN
		{
			uint16_t Address = Opcode & 0x0FFF;
			PC = Address;
			break;
		}
		case 0x2000:	//2NNN: Execute subroutine at address NNN
		{
			uint16_t Address = Opcode & 0x0FFF;
			Stack[SP] = PC;
			++SP;
			PC = Address;
			break;
		}
		case 0x3000:	//3XNN: Skip following instruction if VX == NN
		{
			if (Registers[(Opcode & 0x0F00) >> 8] == (Opcode & 0x00FF))
			{
				PC += 2;
			}
			break;
		}
		case 0x4000:	//4XNN: Skip following instruction if VX != NN
		{
			if (Registers[(Opcode & 0x0F00) >> 8] != (Opcode & 0x00FF))
			{
				PC += 2;
			}
			break;
		}
		case 0x5000:	//5XY0: Skip following instruction if VX == VY
		{
			if (Registers[(Opcode & 0x0F00) >> 8] == Registers[(Opcode & 0x00F0) >> 4])
			{
				PC += 2;
			}
			break;
		}
		case 0x6000:	//6XNN:	Store number NN in register VX
		{
			Registers[(Opcode & 0x0F00) >> 8] = Opcode & 0x00FF;
			break;
		}
		case 0x7000:	//7XNN: Add number NN to register VX
		{
			Registers[(Opcode & 0x0F00) >> 8] += Opcode & 0x00FF;
			break;
		}
		case 0x8000:
		{
			switch (Opcode & 0x000F)
			{
				case 0x0000:	//8XY0: Store the value of register VY in register VX
				{
					Registers[(Opcode & 0x0F00) >> 8] = Registers[(Opcode & 0x00F0) >> 4];
					break;
				}
				case 0x0001:	//8XY1: Set VX to VX OR VY
				{
					uint8_t Byte = Registers[(Opcode & 0x0F00) >> 8] | Registers[(Opcode & 0x00F0) >> 4];
					Registers[(Opcode & 0x0F00) >> 8] = Byte;
					break;
				}
				case 0x0002:	//8XY2: Set VX to VX AND VY
				{
					uint8_t Byte = Registers[(Opcode & 0x0F00) >> 8] & Registers[(Opcode & 0x00F0) >> 4];
					Registers[(Opcode & 0x0F00) >> 8] = Byte;
					break;
				}
				case 0x0003:	//8XY3: Set VX to VX XOR VY
				{
					uint8_t Byte = Registers[(Opcode & 0x0F00) >> 8] ^ Registers[(Opcode & 0x00F0) >> 4];
					Registers[(Opcode & 0x0F00) >> 8] = Byte;
					break;
				}
				case 0x0004:	//8XY4: Add the value of VY to VX and use VF as carry
				{
					uint8_t X = (Opcode & 0x0F00) >> 8;
					uint8_t Y = (Opcode & 0x00F0) >> 4;
					if ((Registers[X] + Registers[Y]) > 0xFF)
					{
						Registers[0xF] = 0x01;
					}
					else
					{
						Registers[0xF] = 0x00;
					}
					Registers[X] = (Registers[X] + Registers[Y]) & 0xFF;
					break;
				}
				case 0x0005:	//8XY5: Subtract the value of VY from VX and use VF as borrow
				{
					uint8_t X = (Opcode & 0x0F00) >> 8;
					uint8_t Y = (Opcode & 0x00F0) >> 4;
					if (Registers[X] > Registers[Y])
					{
						Registers[0xF] = 0x01;
					}
					else
					{
						Registers[0xF] = 0x00;
					}
					Registers[X] -= Registers[Y];
					break;
				}
				case 0x0006:	//8XY6: Store VY >> 1 in VX and save lost bit in VF
				{
					/*
					Registers[0xF] = Registers[(Opcode & 0x00F0) >> 4] & 0x1;
					Registers[(Opcode & 0x0F00) >> 8] = Registers[(Opcode & 0x00F0) >> 4] >> 1;
					break;
					*/

					
					uint8_t Vx = (Opcode & 0x0F00u) >> 8u;
					Registers[0xF] = (Registers[Vx] & 0x1u);
					Registers[Vx] >>= 1;
					break;
			
				}
				case 0x0007:	//8XY7: Set VX to VY minus VX and use VF as borrow
				{
					uint8_t X = (Opcode & 0x0F00) >> 8;
					uint8_t Y = (Opcode & 0x00F0) >> 4;
					if (Registers[X] < Registers[Y])
					{
						Registers[0xF] = 0x01;
					}
					else
					{
						Registers[0xF] = 0x00;
					}
					Registers[X] = Registers[Y] - Registers[X];
					break;
				}
				case 0x000E:	//8XYE: Store VY << 1 in VX and save lost bit in VF
				{
					/*
					Registers[0xF] = (Registers[(Opcode & 0x00F0) >> 4] & 0xF0) >> 4;
					Registers[(Opcode & 0x0F00) >> 8] = Registers[(Opcode & 0x00F0) >> 4] << 1;
					break;
					*/

					
					uint8_t X = (Opcode & 0x0F00) >> 8;
					Registers[0xF] = (Registers[X] & 0x80) >> 7;
					Registers[X] <<= 1;
					break;
					
				}
			}
			break;
		}
		case 0x9000:	//9XY0: Skip following instruction if VX != VY
		{
			if (Registers[(Opcode & 0x0F00) >> 8] != Registers[(Opcode & 0x00F0) >> 4])
			{
				PC += 2;
			}
			break;
		}
		case 0xA000:	//ANNN: Store memory address NNN in register I
		{
			I = Opcode & 0x0FFF;
			break;
		}
		case 0xB000:	//BNNN: Jump to address NNN + V0
		{
			PC = (Opcode & 0x0FFF) + Registers[0x0];
			break;
		}
		case 0xC000:	//CXNN: Set VX to a random number with a mask of NN
		{
			Registers[(Opcode & 0x0F00) >> 8] = randByte(randGen) & (Opcode & 0x00FF);
			break;
		}
		case 0xD000:	//DXYN: Draw sprite at position VX,VY with N bytes of memory starting at I and set VF if collision occurs
		{
			uint8_t X = (Opcode & 0x0F00) >> 8;
			uint8_t Y = (Opcode & 0x00F0) >> 4;
			uint8_t N = Opcode & 0x000F;

			uint8_t Xpos = Registers[X] % 64;
			uint8_t Ypos = Registers[Y] % 32;

			Registers[0xF] = 0;

			for (unsigned int row = 0; row < N; row++)
			{
				uint8_t SpriteByte = Memory[I + row];

				for (unsigned int col = 0; col < 8; col++)
				{
					uint8_t SpritePixel = SpriteByte & (0x80u >> col);
					uint32_t* ScreenPixel = &ScreenPixels[(Ypos + row) * 64 + (Xpos + col)];
					
					if (SpritePixel)
					{
						if (*ScreenPixel == 0xFFFFFFFF)
						{
							Registers[0xF] = 1;
						}

						*ScreenPixel ^= 0xFFFFFFFF;
					}
				}
			}

			break;
		}
		case 0xE000:
		{
			switch (Opcode & 0x000F)
			{
				case 0x000E:	//EX9E: Skip following instruction if key stored in VX is pressed
				{
					if (Keymap[Registers[(Opcode & 0x0F00) >> 8]] == 1)
					{
						PC += 2;
					}
					break;
				}
				case 0x0001:	//EXA1
				{
					if (Keymap[Registers[(Opcode & 0x0F00) >> 8]] == 0)
					{
						PC += 2;
					}
					break;
				}
			}
			break;
		}
		case 0xF000:
		{
			switch (Opcode & 0x00FF)
			{
				case 0x0007:	//FX07: Store current value of delay timer in VX
				{
					Registers[(Opcode & 0x0F00) >> 8] = DelayTimer;
					break;
				}
				case 0x000A:	//FX0A: Wait for a keypress and store the result in VX
				{
					uint8_t X = (Opcode & 0x0F00) >> 8;

					if (Keymap[0])
					{
						Registers[X] = 0;
					}
					else if (Keymap[1])
					{
						Registers[X] = 1;
					}
					else if (Keymap[2])
					{
						Registers[X] = 2;
					}
					else if (Keymap[3])
					{
						Registers[X] = 3;
					}
					else if (Keymap[4])
					{
						Registers[X] = 4;
					}
					else if (Keymap[5])
					{
						Registers[X] = 5;
					}
					else if (Keymap[6])
					{
						Registers[X] = 6;
					}
					else if (Keymap[7])
					{
						Registers[X] = 7;
					}
					else if (Keymap[8])
					{
						Registers[X] = 8;
					}
					else if (Keymap[9])
					{
						Registers[X] = 9;
					}
					else if (Keymap[10])
					{
						Registers[X] = 10;
					}
					else if (Keymap[11])
					{
						Registers[X] = 11;
					}
					else if (Keymap[12])
					{
						Registers[X] = 12;
					}
					else if (Keymap[13])
					{
						Registers[X] = 13;
					}
					else if (Keymap[14])
					{
						Registers[X] = 14;
					}
					else if (Keymap[15])
					{
						Registers[X] = 15;
					}
					else
					{
						PC -= 2;
					}

					break;
				}
				case 0x0015:	//FX15: Set the delay timer to the value of VX
				{
					DelayTimer = Registers[(Opcode & 0x0F00) >> 8];
					break;
				}
				case 0x0018:	//FX18: Set the sound timer to the value of VX
				{
					SoundTimer = Registers[(Opcode & 0x0F00) >> 8];
					break;
				}
				case 0x001E:	//FX1E: Add the value in VX to register I
				{
					I += Registers[(Opcode & 0x0F00) >> 8];
					break;
				}
				case 0x0029:	//FX29: Set I to the address of the sprite data corresponding to hex value in VX
				{
					I = FONT_START_ADDRESS + (5 * Registers[(Opcode & 0x0F00) >> 8]);
					break;
				}
				case 0x0033:	//FX33: Store the binary-coded decimal equivalent of VX at addresses I, I+1 and I+2
				{
					uint8_t X = (Opcode & 0x0F00) >> 8;
					Memory[I] = Registers[X] / 100;
					Memory[I + 1] = (Registers[X] / 10) % 10;
					Memory[I + 2] = (Registers[X] % 100) % 10;
					break;
				}
				case 0x0055:	//FX55: Store registers V0 to VX inclusive in memory starting at I, I is set to I + X + 1
				{
					uint8_t X = (Opcode & 0x0F00) >> 8;
					for (uint8_t i = 0; i <= X; i++)
					{
						Memory[I + i] = Registers[i];
					}
					//I = I + X + 1;
					break;
				}
				case 0x0065:	//FX65: Fill registers V0 to VX inclusive with values from memory starting at I, I is set to I + X + 1
				{
					uint8_t X = (Opcode & 0x0F00) >> 8;
					for (uint8_t i = 0; i <= X; i++)
					{
						Registers[i] = Memory[I + i];
					}
					//I = I + X + 1;
					break;
				}
			}
			break;
		}
	}

	//Update timers
	if (DelayTimer > 0)
	{
		DelayTimer--;
	}
	if (SoundTimer > 0)
	{
		SoundTimer--;
	}
}

std::string Emulator::GetRomName() const
{
	return RomName;
}
