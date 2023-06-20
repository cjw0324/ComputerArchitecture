
struct ControlSignal{
        int RegDest, RegWrite;
        int ALUSrc,ALU;
        int MemWrite,MemRead,MemtoReg;
        int Jump, Branch;
        int DoBranch;
};

struct IFID_latch{
	unsigned int inst;
	int pc, pc4;
	unsigned int valid;
	int BranchPre;
};

struct IDEX_latch{
	unsigned int inst;
	int pc,pc4 ,pcSrc1;
	int opcode;
	int simm,shamt;
	int rs, rt;
	int Rs_value, Rt_value;
	int Baddr;
	unsigned int valid;
	int RegNum;
	int BranchPre;
	struct ControlSignal control;
};

struct EXMM_latch{
	unsigned int inst;
	int pc, pcSrc1, pcSrc2;
	int rs, rt;
	int Rt_value;
	int result;
	unsigned int valid;
	int RegNum;
	struct ControlSignal control;
};

struct MMWB_latch{
	unsigned int inst;
	int pc,npc, pcSrc1, pcSrc2;
	int rs, rt;
	unsigned int valid;
	int RegNum;
	int RegValue;
	struct ControlSignal control;
};

struct Branch_st{
	int pc;
	int Brpc;
	int GO;
};

struct IFID_latch ifid_lat[2];
struct IDEX_latch idex_lat[2];
struct EXMM_latch exmm_lat[2];
struct MMWB_latch mmwb_lat[2];
struct Branch_st Branch_pc[10];

struct Cache{

	int data[16];
	int sca;
	int dirty;
	int tag;
	int valid;
};



