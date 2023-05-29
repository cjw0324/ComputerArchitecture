#include "mips.h"

// sign extend
unsigned int SignExtend(unsigned short immediate)
{
    if (!(immediate & 0x8000))
        return 0x0000ffff & immediate;
    else
        return 0xffff0000 | immediate;
}

// zero extend
unsigned int ZeroExtend(unsigned short immediate)
{
    return 0x0000ffff & immediate;
}

// branch addr
unsigned int BranchAddr(unsigned int PC, unsigned int immediate)
{
    return PC + 4 + (immediate << 2);
}

// jump addr
unsigned int JumpAddr(unsigned int PC, unsigned int address)
{
    return ((PC + 4) & 0xf0000000) | (address << 2);
}

// PC addr
unsigned int PCAddr(unsigned int PC)
{
    if ((inst->opcode == 0x00) && (inst->funct == JALR))
        return PC + 4;
    return PC + MUX(4, 8, JumpLink);
}
