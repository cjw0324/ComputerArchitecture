#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "instruction.h" // variable list

//Declase cycle func
void Inst_Fetch();
void Inst_Decode();
void Execute();
void Memory();
void WriteBack();

void Update_Latch();//Move Latch[0] to Latch[1]
void Init(); //Latch[0] members make 0

int Control(int opcode,int func, struct ControlSignal* control); // Select feature as opcode and func
int ALUOPeration(int r1_value, int r2_value, int ALU); // Calculation values in excute

//Declare memory structure
//Memory has code and data
int Mem [0x100000/4];

//program counter
int PC  =  0;
int CycleNum = 0;// Number of Cycle
int npc = 0; //Next PC
int R[32] = {0,}; // Declare Registar
int k = 0; //Number of Branch
int i = 0; //Variable of Repetition
int MisBranch = 0;
int B_Num = 0;
int Exe_inst = 0;
int Total_inst = 0;
int M_num = 0;
int R_num =0;
int Not_taken_br = 0;
int taken_br = 0;
int J_num = 0;

int main(){
	
	R[31] = 0xffffffff;//initiate R[31] -1
	R[29] = 0x100000; //initiate R[29] StackPoint 0x100000

	FILE *fp = NULL;
	size_t ret = 0;
	int data = 0;
	int index = 0;
	int h1, h2, h3, h4;
	int inst;
	printf("Enter the executable file name :");
        char* file_name = (char*)malloc(sizeof(char)*20);
        scanf("%s", file_name);
	fp = fopen(file_name, "rb");//Open the file
	if (fp == NULL){
		perror("No such input file");
		}

	while(1){//Reversing the values used in the computer
		h1 = 0; 
		h2 = 0;
		h3 = 0;
		h4 = 0;
		inst = 0;
		//reading file to the eof
		ret = fread(&data, sizeof(data), 1, fp);
		if (ret == 0) break;
		printf("orig 0x%08x   ",data);
		
		h1 = ((data & 0xff) << 24);
		h2 = ((data & 0xff00) << 8);
		h3 = (((data & 0xff0000) >> 8) & 0xff00);
		h4 = (((data & 0xff000000) >> 24) & 0xff);

		inst = h1 | h2 | h3 | h4;

		Mem[index/4] = inst;
		printf("[0x%08x] 0x%08x\n",index, Mem[index/4]);
		index = index + 4;
		Total_inst = index/4;
		}

	fclose(fp);

	for(i = 0; i<10; i++){
	bzero(Branch_pc, sizeof(Branch_pc[i]));
	}
//execution begin
	while(1){ //Start Cycle
			Inst_Fetch();
			Inst_Decode();
			Execute();
			Memory();
			WriteBack();
			
			Update_Latch();
			Init();
			CycleNum++;
			
			if(npc == 0xffffffff){
				printf("\n\nProgram is done\n");
				break;
			}//Program is end	
		}
		printf("------------------------------------------------------------------------\n");
		printf("R[2] : %d\n",R[2]);
		printf("The Total number of execution cycles : %d\n",CycleNum-1);
		printf("The Total number of execution Instrtuction : %d\n",Exe_inst);
		printf("Total instruction : %d \n",Total_inst);
		printf("Memory ops : %d\n",M_num);
		printf("Reg ops : %d\n",R_num);
		printf("Not taken branches : %d\n",Not_taken_br);
		printf("Taken branches : %d\n",taken_br);
		printf("Jump : %d\n",J_num);
		printf("B_num : %d Misbranch = %d\n",B_Num,MisBranch); 		
		printf("------------------------------------------------------------------------");
}//main

void Init(){
	bzero(ifid_lat, sizeof(ifid_lat[0]));
	bzero(idex_lat, sizeof(idex_lat[0]));
	bzero(exmm_lat, sizeof(exmm_lat[0]));
	bzero(mmwb_lat, sizeof(mmwb_lat[0]));

}//Latch[0] members make 0

void Inst_Fetch(){

	printf("\n");
	printf("****************cycle[%d]*************************************************************\n",CycleNum);
	
	struct IFID_latch out;
	
	unsigned int inst;
	inst =  Mem[PC/4];
	
	if(inst == 0x00000000 || PC == 0xffffffff){
		
		ifid_lat[0].valid = 0;

		if(inst == 0x00000000)
		{
			printf("index = [0x%08x] PC =  [0x%08x] \n",(CycleNum-1)*4,PC);
			printf("[F] 0x00000000 Nope\n");
			PC = PC + 4;
		}
		return;
	}//Nope

	out.inst = inst;
	out.pc = PC;
	out.pc4 = PC + 4;
			
	int BranchPre = 0; // variable Determine branch predicted in execute
	int check, BranchTarget; // variable BranchGo :  Branch address  Check : Is it Branch
	
	for(i = 0; i<10; i++){
		if(PC == Branch_pc[i].pc && Branch_pc[i].pc){
			BranchTarget = Branch_pc[i].Brpc;
			check = 1;
			break;
		}
		else{
			check = 0;
		}
	}// Check when do branch
	
	BranchPre = (check == 1 && Branch_pc[i].GO == 1)? 1: 0;
	PC = (BranchPre == 1)? BranchTarget : out.pc4; 
	
	out.BranchPre = BranchPre;

	printf("index = [0x%08x] PC =  [0x%08x] \n",CycleNum*4,PC);
	printf("[F] inst = 0x%08x\n", inst);

	out.valid = 1; //Do next cycle
	ifid_lat[0] = out; // init latch

	return;	
}

void Inst_Decode(){

	struct IFID_latch in = ifid_lat[1]; 
	struct IDEX_latch out;

	if(in.valid == 0)
	{
		idex_lat[0].valid = 0;
		return;
	}// Previous cycle is disabled -> next cycle is disabled to

	int inst = in.inst;

	//decode the variable
	int opcode = (inst & 0xfc000000) >> 26;
	int rs	 = (inst & 0x03e00000) >> 21;
	int rt	 = (inst & 0x001f0000) >> 16;
	int rd	 = (inst & 0x0000f800) >> 11;
	int func	 = (inst & 0x0000003f) >> 0;
	int imm	 = (inst & 0x0000ffff) >>  0;
	int simm 	 = ((imm >> 15) == 1) ? (0xffff0000 | imm)  : imm;
	int shamt 	 = (inst & 0x000007c0) >> 6;
	int address = (inst & 0x03ffffff) >> 0; 
	int jumpaddr = (address << 2)|(PC & 0xf0000000) ;
	int branchaddr = ((imm >> 15) == 1) ? ((0x3fff0000 | imm) << 2 ) : (imm << 2) ;

	Control(opcode,func,&out.control);

	if(opcode == 0xd)//In gcd.bin file , li opcode is 0xd, need initiate value
	simm = imm;
	
	out.inst = in.inst;
	out.pc = in.pc;
	out.pc4 = in.pc4;
	out.opcode = opcode;
	out.shamt = shamt;
	out.simm = simm;
	out.rs = rs;
	out.Rs_value = (rs == mmwb_lat[1].RegNum && rs && mmwb_lat[1].control.RegWrite)? mmwb_lat[1].RegValue: R[rs];
	out.rt = rt;
	out.Rt_value = (rt == mmwb_lat[1].RegNum && rt && mmwb_lat[1].control.RegWrite)? mmwb_lat[1].RegValue: R[rt];
	
	if(out.control.RegWrite== 1){ // Determine Register to write
		if(out.control.RegDest == 0)
			out.RegNum= rt;
		else 
		{
			if(out.control.RegDest == 1)
				out.RegNum = rd;
			else 
				out.RegNum = 31;
		}	
	R_num++;	
	}
	else
	out.RegNum = 0;

	if(out.control.Jump == 1){ //When Jump Occurs
		if(opcode==0x0&&func==0x08)// jr
		{
			if(mmwb_lat[1].RegNum ==rs)  
			R[rs] = mmwb_lat[1].RegValue;

			out.pcSrc1 = R[rs];
			PC = out.pcSrc1;
		}
		else{ // j
			out.pcSrc1 = jumpaddr;
			PC = out.pcSrc1;
		}
	J_num++;
	}

	out.Baddr = branchaddr + 4 + in.pc;
	out.BranchPre = in.BranchPre;
	
	printf("[D] inst = 0x%08x  opcode(0x%x) rs : %d(0x%08x) rt : %d(0x%08x) rd : %d func(%d) simm  0x%08x (%d)\n",in.inst,opcode, rs,out.Rs_value, rt,out.Rt_value, rd,func,simm,simm);
	
	out.valid = 1;
	idex_lat[0] = out;
	return;
}//Decode

void Execute(){

	struct IDEX_latch in = idex_lat[1];
	struct EXMM_latch out;
	
	if(idex_lat[1].valid == 0){
		exmm_lat[0].valid = 0;
		return ;
	}
	Exe_inst++;
	if(in.control.Jump == 0){ //When jump not occurs
	
	int r1_value, r2_value, R_r2_value;

	if(idex_lat[1].rs == exmm_lat[1].RegNum&&exmm_lat[1].RegNum&&exmm_lat[1].control.RegWrite){
		r1_value = exmm_lat[1].result;
		}
	else if(idex_lat[1].rs == mmwb_lat[1].RegNum&&mmwb_lat[1].control.RegWrite&&mmwb_lat[1].RegNum){
			r1_value = mmwb_lat[1].RegValue;
	}
	else
	r1_value = in.Rs_value;
	
	if(idex_lat[1].rt == exmm_lat[1].RegNum && exmm_lat[1].RegNum && exmm_lat[1].control.RegWrite){
		r2_value = exmm_lat[1].result;
		}
	else if(idex_lat[1].rt == mmwb_lat[1].RegNum&&mmwb_lat[1].control.RegWrite&&mmwb_lat[1].RegNum){
			r2_value = mmwb_lat[1].RegValue;

	}else
	r2_value = in.Rt_value;

	R_r2_value = r2_value;// Orignal r2_value before changed

	if(in.opcode == 0x4){	//beq
		if(r1_value == r2_value)
		{
		in.control.DoBranch = 1;
		taken_br++;
		}
		else
		Not_taken_br++;

	}
		
	if(in.opcode == 0x5){	//bne
		if(r1_value != r2_value)
		{
		in.control.DoBranch = 1;
		taken_br++;
		}
		else
		Not_taken_br++;
	}

	if(in.control.DoBranch == 0 && in.BranchPre == 1){	//Not do branch but, have a past do branch (last)
		exmm_lat[0].valid = 0;
		ifid_lat[0].valid = 0;
		PC = in.pc4;
		MisBranch++;
	}
	else if(in.control.DoBranch == 1 && in.BranchPre == 0){ //Do branch but, not have a past do branch (first)
		exmm_lat[0].valid = 0;
		ifid_lat[0].valid = 0;
		PC = in.Baddr;	
		MisBranch++;
	}

	if(in.control.Branch){ 	//Check and put in Branch PC value to each Branch structure
		
		B_Num++;
	
		int standard = 0;	//variable determine when Brnach do in past
		for(i=0; i<=k; i++){	
			if(Branch_pc[i].pc == in.pc )
			{
				standard = 1;
				break;
			}
			else
				standard = 0;
		}

		if(standard == 0)	{
			Branch_pc[k].pc = in.pc;
			Branch_pc[k].Brpc = in.Baddr;
			k++;
		}
	
		if(in.control.DoBranch)
			Branch_pc[k-1].GO = 1;
		else
			Branch_pc[k-1].GO = 0;
	}

	if(in.control.ALUSrc == 1) // Determine r2_value to simm
	r2_value = in.simm;
	
	if(in.control.ALU == 6 || in.control.ALU == 7){	//when srl sll, set value
		r1_value = r2_value;
		r2_value= in.shamt;
	}

	int result = ALUOPeration(r1_value,r2_value, in.control.ALU); // Execute

	printf("[E] inst = 0x%08x  r1_value = 0x%08x  r2_value = 0x%08x  in.control.ALU = %d, Result = 0x%x\n",in.inst, r1_value, r2_value,in.control.ALU, result);
	
	out.result = result;
	out.Rt_value = R_r2_value;
	
	}//When not occurs jump

	if(in.opcode == 0x3) // JAL opcode
	out.result = in.pc + 8;

	out.inst = in.inst;
	out.pc = in.pc;
	out.pcSrc1 = in.pcSrc1;
	out.pcSrc2 = (in.control.DoBranch) ? in.Baddr : in.pc4;
	out.rs = in.rs;
	out.rt = in.rt;
	out.control = in.control;
	out.RegNum = in.RegNum;
	
	out.valid = 1;
	exmm_lat[0] = out;

	return ;
}//execute

void Memory(){

	struct EXMM_latch in = exmm_lat[1];
	struct MMWB_latch out;

	int a = 0; //variable determine what kinds

	if(in.valid == 0){
		mmwb_lat[0].valid = 0;
		return ;
	}

	if(in.control.MemRead == 1){
		out.RegValue = Mem[in.result/4];
		printf("[M] inst = 0x%08x  lw  0x%08x Mem[0x%08x]\n",out.RegValue,in.inst,in.result/4);
		a=1;
		M_num++;
	} //loadward

	if(in.control.MemWrite == 1){
		Mem[in.result/4] = in.Rt_value;
		printf("[M] inst = 0x%08x  sw 0x%08x  Mem[0x%08x]\n",in.Rt_value,in.inst,in.result/4);
		a=2;
		M_num++;
	} //store ward

	if(a == 0) 
		out.RegValue = in.result;
  
	out.inst = in.inst;
	out.pc = in.pc;
	out.pcSrc1 = in.pcSrc1;
	out.pcSrc2 = in.pcSrc2; 
	out.rs = in.rs;
	out.rt = in.rt;
	out.control = in.control;
 	out.RegNum = in.RegNum;

	out.valid = 1;
	mmwb_lat[0] = out;
	return;	
}//Memory

void WriteBack(){

	struct MMWB_latch in = mmwb_lat[1];
	
	if(in.control.Jump) 
		npc = in.pcSrc1;
	else 
		npc = in.pcSrc2;

	if(in.valid == 0 || in.control.RegWrite == 0)
	return ;

	R[in.RegNum] = in.RegValue;

	printf("[WB] inst = 0x%08x  R[%d] = 0x%08x\n",in.inst, in.RegNum, R[in.RegNum]);
	return ;
}//writeback
	
int Control(int opcode, int func, struct ControlSignal* C){
	
	C->DoBranch = 0;
	
	switch(opcode){		//RegDest

		case 0x0:	//Type R
		C->RegDest = 1;//RegWrite = R[rd];
		break;

		case 0x3:	//Jal
		C->RegDest = 2;//Regwrite = R[31];
		break;
	
		default:	//Type I
		C->RegDest = 0; // Regwrite = R[rt]
		break;		
		}

	switch(opcode){		//ALUSrc

		case 0x0:	//Type R
		case 0x4:	//beq
		case 0x5:	//bne
		C->ALUSrc = 0;// r2_value = R[rt]
		break;

		default:
		C->ALUSrc = 1 ;//r2_value = imm
		break;		
		
	}

	switch(opcode){		//load memory
		
		case 0x24://lbu
		case 0x25://lhu
		case 0x30://ll
		case 0x23://lw
		C->MemRead = 1;// read value
		break;

		default:
		C->MemRead = 0;
		break;
	}

	switch(opcode){		//store memory

		case 0x28://sb
		case 0x38://sc
		case 0x29://sh
		case 0x2b://sw
		C->MemWrite = 1;// write value
		break;

		default:
		C->MemWrite = 0;
		break;
	}

	switch(opcode){		//RegWrite
	
		case 0x28://sb
		case 0x38://sc
		case 0x29://sh
		case 0x2b://sw
		case 0x2://j
		case 0x4://beq
		case 0x5://bne
		C->RegWrite = 0;// Not write in register
		break;

		default:
		if(opcode==0x0&&func==0x08) // jr
			C->RegWrite=0;
		else
			C->RegWrite=1;
		break;
	}

	switch(opcode){		//Jump

		case 0x2://j
		case 0x3://jal
		C->Jump = 1;
		break;
	
		default:
		if(opcode==0x0&&func==0x08)// jr	
			C->Jump = 1;
		else
			C->Jump = 0;
		break;
	}

	switch(opcode){		//Branch

		case 0x4:// beq
		case 0x5:// bne
		C->Branch = 1;
		break;

		default:
		C->Branch = 0;
		break;
	}

	switch(opcode){		//Determine ALU

		case 0x0:
			switch(func){
			
				case 0x20://add
				case 0x21://addu
				C->ALU = 1;//+
				break;
				
				case 0x24://and
				C->ALU = 2;//&
				break;
		
				case 0x27://nor
				C->ALU = 3;//!|
				break;

				case 0x25://or
				C->ALU = 4;// |
				break;

				case 0x2a://slt
				case 0x2b://sltu
				C->ALU = 5;// <
				break;

				case 0x00:// sll
				C->ALU = 6;// <<
				break;

				case 0x02://srl
				C->ALU = 7;// >>
				break;

				case 0x23://subu
				case 0x22://sub
				C->ALU = 9;// -			
				break;
		}
		break;
		
	case 0x8://addo
	case 0x9://addiu
	C->ALU = 1; // +
	break;

	case 0xc://andi
	C->ALU = 2; //&
	break;

	case 0xd://ori
	C->ALU = 4; // |
	break;

	case 0xf://lui
	C->ALU = 8;//lui imm<<16
	break;
	
	case 0xa://slti
	case 0xb://sltiu
	C->ALU = 5; // <
	break;
	
	default:
	C->ALU = 0;
	break;

	}
}


int ALUOPeration(int r1_value, int r2_value, int ALU){

	int value;
	
	switch(ALU){
		case 0:
		case 1: 
		value = r1_value + r2_value; 
		break;

		case 2:
		value = r1_value & r2_value;
 		break;

		case 3:
		value = !(r1_value | r2_value);
		break;

		case 4:
		value = r1_value | r2_value;
 		break;

		case 5:
		value = (r1_value < r2_value) ? 1:0; 
		break;

		case 6:
		value = r1_value << r2_value; 
		break;

		case 7:
		value = r1_value >> r2_value; 
		break;

		case 8:
		value = r2_value << 16; 
		break; //lui

		case 9:
		value = r1_value - r2_value; 
		break; //sub
	}
	return value;
}

void Update_Latch(){
	ifid_lat[1] = ifid_lat[0];
	idex_lat[1] = idex_lat[0];
	exmm_lat[1] = exmm_lat[0];
	mmwb_lat[1] = mmwb_lat[0];
}
