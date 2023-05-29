#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "register.h"
#include "instruction.h"

char *PROGRAM;
#define REGSIZE 32
#define MEMSIZE 0x1000000 // 16MB

// instruction type
enum inst_type
{
    J_type,
    I_type,
    R_type
};

// instruction structure
typedef struct instruction
{
    unsigned char opcode;
    unsigned char rs;
    unsigned char rt;
    unsigned char rd;
    unsigned char shamt;
    unsigned char funct;
    unsigned int immediate;
    unsigned int address;
} instruction;

// control unit variables
bool RegDst;
bool SignEx;
bool Shift;
bool ALUSrc;
bool MemtoReg;
bool RegWrite;
bool MemRead;
bool MemWrite;
bool Branch;
bool Jump;
bool JumpReg;
bool JumpLink;
bool Rtype;
bool Equal;
bool RA;
unsigned char ALUOp;

// addr
unsigned int PCAddr(unsigned int PC);
unsigned int SignExtend(unsigned short immediate);
unsigned int ZeroExtend(unsigned short immediate);
unsigned int JumpAddr(unsigned int PC, unsigned int address);
unsigned int BranchAddr(unsigned int PC, unsigned int immediate);

// control unit
void CU_Init();
void CU_Operation(unsigned char opcode, unsigned int funct);

// instruction memory
void IM_Init();
unsigned int IM_ReadMemory(unsigned int ReadAddress);

// registers
void RF_Init();
void RF_Read(unsigned char RdReg1, unsigned char RdReg2);
void RF_Write(unsigned char WrtReg, unsigned int WrtData, bool WrtEnable);

// ALU
unsigned char ALU_Control(unsigned char ALUOp, unsigned char opcode);
unsigned int ALU_Operation(unsigned char ALU_Control, unsigned int operand1, unsigned int operand2);

// data memory
void DM_Init();
void DM_MemoryAccess(unsigned int Address, int size, unsigned int WriteData, bool MemRead, bool MemWrite);

// main
void init_all();
void terminate();
void Inst_Decode();
void Print_Decode();
void Print_Execute();
unsigned int MUX(unsigned int IN1, unsigned int IN2, bool S);

// instruction count
int inst_count;
int R_count;
int I_count;
int J_count;
int M_count;
int B_count;

// CPU variables
bool bcond;                        // branch condition
unsigned int IR;                   // instruction register
unsigned int PC;                   // program counter
instruction *inst;                 // instruction
unsigned char ALUControl;          // output of ALU control units
unsigned int ALUResult;            // output of ALU unit opration
unsigned int ReadData;             // output of data memory
unsigned int ReadData1;            // output of RF
unsigned int ReadData2;            // output of RF
unsigned int Register[REGSIZE];    // register
unsigned char InstMemory[MEMSIZE]; // instruction memory
unsigned char DataMemory[MEMSIZE]; // data memory
