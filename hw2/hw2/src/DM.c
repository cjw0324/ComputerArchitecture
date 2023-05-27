#include "mips.h"

// initialize the data memory
void DM_Init()
{
	for (int i = 0; i < MEMSIZE; i++)
		DataMemory[i] = 0;
}

// data memory access
void DM_MemoryAccess(unsigned int Address, int size, unsigned int WriteData, bool MemRead, bool MemWrite)
{

	// memory read operation
	if ((MemRead == true) && (MemWrite == false))
	{
		unsigned int result = 0;
		for (int i = 0; i < (size / 8); i++)
		{
			result = result << 8;
			result = result | DataMemory[Address + i];
		}
		ReadData = result;
	}

	// memory write operation
	if ((MemRead == false) && (MemWrite == true))
	{
		for (int i = (size / 8) - 1; i >= 0; i--)
		{
			DataMemory[Address + i] = WriteData & 0xff;
			WriteData = WriteData >> 8;
		}
	}
}
