#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int memory[0x40000];

int reg[32];
unsigned int pc;
unsigned int npc;
int Forward[3];
int Forward_val[3] ;
int ctrl_dep;
unsigned int branch_pc_slot[10];
int branch_target_slot[10];
int branch_taken_slot[10][2];
unsigned int jump_pc_slot[5];
int jump_target_slot[5];

typedef struct{
	int instruction;
	int address;
	int opcode;
	int rs;
	int rt;
	int rd;
	int immediate;
	int funct;
	int shamt;
}d_instruction;

typedef struct{
	int RegDst;
	int Jump;
	int Branch;
	int MemRead;
	int MemtoReg;
	int ALUOp;
	int MemWrite;
	int ALUSrc;
	int RegWrite;
}ctrl_signal;

typedef struct{
	int alu_ctrl_num;
}ALU_ctrl;

typedef struct{
	int Read_register1;
	int Read_register2;
	int Write_register;
	int Write_data;
	int Read_data1;
	int Read_data2;
}Registers;

typedef struct{
	int Address;
	int Write_data;
	int Read_data;
}Data_MEM;

typedef struct{
	int ALU_result;
}ALU_val;

typedef struct{
	unsigned int pc_val;
	Registers R_v;
	d_instruction d_i;
	ctrl_signal c_s;
	ALU_ctrl A_c;
	Data_MEM D_M;
	ALU_val A_v;
}Latch;

typedef struct{
	int cycle_num;
	int R_type_num;
	int I_type_num;
	int J_type_num;
	int MEM_Access_num;
	int branch_num;
	int branch_satisfied_num;
	int branch_predict_success;
	int branch_predict_fail;
}print_num;

int jump_slot_config(unsigned int pc_val);
void jump_pc_slot_in(unsigned int pc_val, int jump_address);
int branch_flag_config(unsigned int pc_val);
void branch_pc_slot_in(unsigned int pc_val, int branch_address, int taken_val);
void bubble_generator(Latch* L);
void Latch_set(Latch* IF_ID, Latch* ID_EX, Latch* EX_MEM, Latch* MEM_WB);
void print_num_set(print_num* p_n);
void reg_set();
void ctrl_sig_set(ctrl_signal* c_s);
int MUX(int input1, int input2, int sig);
int store2memory();
void Reg_val(int Read_reg1, int Read_reg2, int Write_reg,  Registers* R_v);
void ctrl(d_instruction* d_i, ctrl_signal* c_s, print_num* p_n);
void ALU_ctrl_sig(ctrl_signal* c_s, int opcode, int funct, ALU_ctrl* A_c);
void ALU(int input1, int input2, int ALU_ctrl, ALU_val* A_v, print_num* p_n);
void Forwarding_unit_in(Latch* L, int i);
int F_mux(int r, int data);
///////////////////////////////////////////////////////////////
void pipeline();
void inst_fetch(Latch* IF_ID, Latch* EX_MEM);
void inst_decode(Latch* IF_ID, print_num* p_n);
void execution(Latch* ID_EX, print_num* p_n);
void MEM_Access(Latch* EX_MEM, print_num* p_n);
void Write_Back(Latch* MEM_WB);
//////////////////////////////////////////////////////////////////
void print_cycle_end(print_num p_n);
void print_cycle();
void print_stage(Latch* L, int i);

////////////////////////////////////////////////////////////////
void main(){
	reg_set();
	
	if(store2memory());
	else return ;
	pipeline();
	
	return ;
}

int jump_slot_config(unsigned int pc_val){
	for(int i=0; i<10; i++){
		if(jump_pc_slot[i] == 0) return 0;
		else if(jump_pc_slot[i] == pc_val) return 1;
	}
	return 0;
}

void jump_pc_slot_in(unsigned int pc_val, int jump_address){
	for(int i=0; i<10; i++){
		if(jump_pc_slot[i] == pc_val) return ;
		else if(jump_pc_slot[i] == 0){
			jump_pc_slot[i] = pc_val;
			jump_target_slot[i] = jump_address;
			return ;
		}
	}
}

int branch_flag_config(unsigned int pc_val){
	for(int i=0; i<10; i++){
		if(branch_pc_slot[i] == pc_val){
			if(branch_taken_slot[i][1] == 1){
				if(branch_taken_slot[i][0] == 1) return 3;
				else return 2;
			}
			else{
				if(branch_taken_slot[i][0] == 1) return 1;
				else return 0;
			}
		}
	}
	return 0;
}

void branch_slot_in(unsigned int pc_val, int branch_address, int taken_val){
	for(int i=0; i<10; i++){
		if(branch_pc_slot[i] == pc_val){
			if(taken_val == 1){
				switch(branch_flag_config(pc_val)){
					case 0:
						branch_taken_slot[i][0] = 1;
						branch_taken_slot[i][1] = 0;
						break;
					case 1:
						branch_taken_slot[i][0] = 0;
						branch_taken_slot[i][1] = 1;
						break;
					case 2:
						branch_taken_slot[i][0] = 1;
						branch_taken_slot[i][1] = 1;
						break;
					case 3:
						branch_taken_slot[i][0] = 1;
						branch_taken_slot[i][1] = 1;
						break;
				}
			}
			else{
				switch(branch_flag_config(pc_val)){
					case 0:
						branch_taken_slot[i][0] = 0;
						branch_taken_slot[i][1] = 0;
						break;
					case 1:
						branch_taken_slot[i][0] = 0;
						branch_taken_slot[i][1] = 0;
						break;
					case 2:
						branch_taken_slot[i][0] = 1;
						branch_taken_slot[i][1] = 0;
						break;
					case 3:
						branch_taken_slot[i][0] = 0;
						branch_taken_slot[i][1] = 1;
						break;
				}
			}
			return ;
		}
		else if(branch_pc_slot[i] == 0){
			branch_pc_slot[i] = pc_val;
			branch_target_slot[i] = branch_address;
			if(taken_val == 1){
				branch_taken_slot[i][0] = 1;
				branch_taken_slot[i][1] = 1;
			}
			else{
				branch_taken_slot[i][0] = 0;
				branch_taken_slot[i][1] = 0;
			}
			return ;
		}
	}
}

void bubble_generator(Latch* L){
	memset(L, 0, sizeof(Latch));
}

void Latch_set(Latch* IF_ID, Latch* ID_EX, Latch* EX_MEM, Latch* MEM_WB){
	memset(IF_ID, 0, sizeof(Latch));
	memset(ID_EX, 0, sizeof(Latch));
	memset(EX_MEM, 0, sizeof(Latch));
	memset(MEM_WB, 0, sizeof(Latch));
}

void print_num_set(print_num* p_n){
	p_n->cycle_num = 1;
	p_n->R_type_num = 0;
	p_n->I_type_num = 0;
	p_n->J_type_num = 0;
	p_n->MEM_Access_num = 0;
	p_n->branch_num = 0;
	p_n->branch_satisfied_num = 0;
	p_n->branch_predict_success = 0;
	p_n->branch_predict_fail = 0;
}

void reg_set(){
	for(int n=0; n<32; n++){
		reg[n] = 0;
	}
	reg[29] = 0x100000;
	reg[31] = 0xffffffff;
	pc = 0x0;
	return ;
}

void ctrl_sig_set(ctrl_signal* c_s){
	c_s->RegDst = 0;
	c_s->Jump = 0;
	c_s->Branch = 0;
	c_s->MemRead = 0;
	c_s->MemtoReg = 0;
	c_s->ALUOp = 0;
	c_s->MemWrite = 0;
	c_s->ALUSrc = 0;
	c_s->RegWrite = 0;
}

int MUX(int input1, int input2, int sig){
	if(sig) return input2;
	else return input1;
}

int store2memory(){
	FILE* fp = NULL;
	unsigned int data = 0;
	int m = 0;
	char* file_name;
	file_name = (char*)malloc(sizeof(char)*20);

	scanf("%s", file_name);
	
	fp = fopen(file_name, "rb");
	if(fp == NULL){
		printf("ERROR opening file!\n");
		return 0;
	}
	while(fread(&data, sizeof(int), 1, fp)){
	memory[m] = (((data & 0x000000ff) << 24) & 0xff000000) | (((data & 0x0000ff00) << 8) &0x00ff0000) | 
	(((data & 0x00ff0000) >> 8) & 0xff00) | (((data & 0xff000000) >> 24) & 0xff);
	m++;
	}
	fclose(fp);
	return 1;
}

void Reg_val(int Read_reg1, int Read_reg2, int Write_reg,  Registers* R_v){
	R_v->Read_register1 = Read_reg1;
	R_v->Read_register2 = Read_reg2;
	R_v->Write_register = Write_reg;
	R_v->Read_data1 = reg[R_v->Read_register1];
	R_v->Read_data2 = reg[R_v->Read_register2];
	return ;
}

void ctrl(d_instruction* d_i, ctrl_signal* c_s, print_num* p_n){
	ctrl_sig_set(c_s);

	if(d_i->opcode == 0){
		c_s->RegDst = 1;
		c_s->ALUOp = 1;
		if((d_i->rs == 0x0) && (d_i->rd == 0x0) && (d_i->rt == 0x0) && (d_i->shamt == 0x0) && (d_i->funct == 0));
		else{
			p_n->R_type_num++;
		}
	}
	else{
		if((d_i->opcode == 0x2) || (d_i->opcode == 0x3)){
			p_n->J_type_num++;
		}
		else{
			p_n->I_type_num++;
		}
	}
	
	if((d_i->opcode != 0) && (d_i->opcode != 0x4) && (d_i->opcode != 0x5)) c_s->ALUSrc = 1;
	else ;
	
	if((d_i->opcode == 0x23) || (d_i->opcode == 0x24) || (d_i->opcode == 0x25) || (d_i->opcode == 0x30)) c_s->MemtoReg = 1;
	else ;

	if((d_i->opcode != 0x2b) && (d_i->opcode != 0x4) && (d_i->opcode != 0x5) && (d_i->opcode != 0x2) && (!((d_i->opcode == 0x0) && (d_i->funct == 0x8))) && (d_i->opcode != 0x28) && (d_i->opcode != 0x38) && (d_i->opcode != 0x29)) c_s->RegWrite = 1;
	else ;

	if((d_i->opcode == 0x23) || (d_i->opcode == 0x24) || (d_i->opcode == 0x25) || (d_i->opcode == 0x30)) c_s->MemRead = 1;
	else ;
	
	if((d_i->opcode == 0x2b) || (d_i->opcode == 0x28) || (d_i->opcode == 0x38) || (d_i->opcode == 0x29)) c_s->MemWrite = 1;
	else ;

	if((d_i->opcode == 0x4) || (d_i->opcode == 0x5) || (d_i->opcode == 0x20)){
		c_s->Branch = 1;
		p_n->branch_num++;
	}
	else ;

	if(((d_i->opcode == 0x0) && ((d_i->funct == 0x8) || (d_i->funct == 0x9))) || (d_i->opcode == 0x2) || (d_i->opcode == 0x3)) c_s->Jump = 1;
	else ;
}

void ALU_ctrl_sig(ctrl_signal* c_s, int opcode, int funct, ALU_ctrl* A_c){
	if(c_s->ALUOp){
		switch(funct){
			case 0x20:
			case 0x21:
				A_c->alu_ctrl_num = 1;
				return ;
			case 0x24:
				A_c->alu_ctrl_num = 2;
				return ;
			case 0x25:
				A_c->alu_ctrl_num = 3;
				return ;
			case 0x27:
				A_c->alu_ctrl_num = 10;
				return ;
			case 0x2a:
			case 0x2b:
				A_c->alu_ctrl_num = 4;
				return ;
			case 0x00:
				A_c->alu_ctrl_num = 5;
				return ;
			case 0x02:
			case 0x03:
				A_c->alu_ctrl_num = 6;
				return ;
			case 0x22:
			case 0x23:
				A_c->alu_ctrl_num = 7;
				return ;
			case 0x8:
			case 0x9:
				A_c->alu_ctrl_num = 12;
				return ;
			default:
				return ;
		}
	}
	else{
		switch(opcode){
			case 0x8:
			case 0x9:
			case 0x24:
			case 0x25:
			case 0x30:
			case 0x23:
			case 0x28:
			case 0x38:
			case 0x29:
			case 0x2b:
				A_c->alu_ctrl_num = 1;
				return ;
			case 0xc:
				A_c->alu_ctrl_num = 2;
				return ;
			case 0x4:
			case 0x20:
				A_c->alu_ctrl_num = 8;
				return ;
			case 0x5:
				A_c->alu_ctrl_num = 9;
				return ;
			case 0xd:
				A_c->alu_ctrl_num = 3;
				return;
			case 0xa:
			case 0xb:
				A_c->alu_ctrl_num = 4;
				return ;
			case 0xf:
				A_c->alu_ctrl_num = 11;
				return ;
			default:
				return ;
		}
	}
}

void ALU(int input1, int input2, int ALU_ctrl, ALU_val* A_v, print_num* p_n){
	switch(ALU_ctrl){
		case 0:
			A_v->ALU_result = 0;
			return ;
		case 1:
			A_v->ALU_result = input1 + input2;
			return ;
		case 2:
			A_v->ALU_result = input1 & input2;
			return ;
		case 3:
			A_v->ALU_result = input1 | input2;
			return ;
		case 4:
			if(input1<input2){
				A_v->ALU_result = 1;
			}
			else{
				A_v->ALU_result = 0;
			}
			return ;
		case 5:
			A_v->ALU_result = input1 << input2;
			return ;
		case 6:
			A_v->ALU_result = input1 >> input2;
			return ;
		case 7:
			A_v->ALU_result = input1 - input2;
			return ;
		case 8:
			if(input1 == input2){
				A_v->ALU_result = 1;
				p_n->branch_satisfied_num++;
			}
			else A_v->ALU_result = 0;
			return ;
		case 9:
			if(input1 != input2){
				A_v->ALU_result = 1;
				p_n->branch_satisfied_num++;
			}
			else A_v->ALU_result = 0;
			return ;
		case 10:
			A_v->ALU_result = ~(input1 | input2);
			return ;
		case 11:
			A_v->ALU_result = (input2 << 16) & 0xffff0000;
			return ;
		case 12:
			A_v->ALU_result = input1;
		default:
			return ;
	}
}

void Forwarding_unit_in(Latch* L, int i){
	if(i == 1){
		Forward[2] = Forward[1];
		Forward[1] = Forward[0];
		Forward[0] = L->R_v.Write_register;
		Forward_val[2] = Forward_val[1];
		Forward_val[1] = Forward_val[0];
		Forward_val[0] = L->D_M.Read_data;
		return ;
	}
	else{
		if(L->c_s.RegWrite == 1){
			if((L->c_s.MemtoReg == 0) && (L->c_s.Jump == 0)){
				Forward[2] = Forward[1];
				Forward[1] = Forward[0];
				Forward[0] = L->R_v.Write_register;
				Forward_val[2] = Forward_val[1];
				Forward_val[1] = Forward_val[0];
				Forward_val[0] = L->A_v.ALU_result;
				return ;
			}
			else if(L->c_s.Jump){
				Forward[2] = Forward[1];
				Forward[1] = Forward[0];
				Forward[0] = L->R_v.Write_register;
				Forward_val[2] = Forward_val[1];
				Forward_val[1] = Forward_val[0];
				Forward_val[0] = L->pc_val+8;
				return ;
			}
			return ;
		}
		else{
			Forward[2] = Forward[1];
			Forward[1] = Forward[0];
			Forward[0] = 0;
			Forward_val[2] = Forward_val[1];
			Forward_val[1] = Forward_val[0];
			Forward_val[0] = 0;
			return ;
		}
	}
}

int F_mux(int r, int data){
	for(int i=0; i<3; i++){
		if(Forward[i] == r){
			return Forward_val[i];
		}
	}
	return data;
}

void pipeline(){
	print_num p_n;
	print_num_set(&p_n);
	Latch IF_ID[2];
	Latch ID_EX[2];
	Latch EX_MEM[2];
	Latch MEM_WB[2];
	for(int i=0; i<2; i++){
		Latch_set(&IF_ID[i], &ID_EX[i], &EX_MEM[i], &MEM_WB[i]);
	}
	while(pc != 0xffffffff){
		printf("--------------------cycle %d--------------------\n", p_n.cycle_num);
		inst_fetch(&IF_ID[0], &EX_MEM[1]);
		inst_decode(&IF_ID[1], &p_n);
		ID_EX[0] = IF_ID[1];
		execution(&ID_EX[1], &p_n);
		EX_MEM[0] = ID_EX[1];
		MEM_Access(&EX_MEM[1], &p_n);
		MEM_WB[0] = EX_MEM[1];
		Write_Back(&MEM_WB[1]);
		IF_ID[1] = IF_ID[0];
		ID_EX[1] = ID_EX[0];
		EX_MEM[1] = EX_MEM[0];
		MEM_WB[1] = MEM_WB[0];
		p_n.cycle_num++;
		print_cycle();
	}
	print_cycle_end(p_n);
}

void inst_fetch(Latch* IF_ID, Latch* EX_MEM){
	if(ctrl_dep != 1){
		IF_ID->pc_val = npc;
		IF_ID->d_i.instruction = memory[npc/4];
	}
	if((EX_MEM->d_i.opcode == 0x0) && ((EX_MEM->d_i.funct == 0x8) || (EX_MEM->d_i.funct == 0x9))){
		npc = EX_MEM->R_v.Read_data1;
	}
	else{
		npc = MUX(MUX(IF_ID->pc_val+4, (EX_MEM->d_i.immediate << 2)+(EX_MEM->pc_val+4), (EX_MEM->c_s.Branch & EX_MEM->A_v.ALU_result)), EX_MEM->d_i.address, EX_MEM->c_s.Jump);
	}
	if(ctrl_dep == 1){
		if(npc == 0xffffffff){
			bubble_generator(IF_ID);
			print_stage(IF_ID, 1);
			pc = npc;
			return ;
		}
		IF_ID->pc_val = npc;
		IF_ID->d_i.instruction = memory[npc/4];
		npc = npc + 4;
	}
	if(branch_flag_config(IF_ID->pc_val) > 1){
		for(int i=0; i<10; i++){
			if(branch_pc_slot[i] == IF_ID->pc_val){
				npc = branch_target_slot[i];
			}
		}
	}
	if(jump_slot_config(IF_ID->pc_val)){
		for(int i=0; i<10; i++){
			if(jump_pc_slot[i] == IF_ID->pc_val){
				npc = jump_target_slot[i];
			}
		}
	}
	print_stage(IF_ID, 1);
}

void inst_decode(Latch* IF_ID, print_num* p_n){
	if(ctrl_dep == 1){
		bubble_generator(IF_ID);
		print_stage(IF_ID, 2);
		return ;
	}
	else if(ctrl_dep == 2){
		bubble_generator(IF_ID);
		print_stage(IF_ID, 2);
		return ;
	}
	IF_ID->d_i.address = ((IF_ID->d_i.instruction & 0x03ffffff) << 2) | ((IF_ID->pc_val+4) & 0xf0000000);
	IF_ID->d_i.opcode = (IF_ID->d_i.instruction & 0xfc000000) >> 26;
	IF_ID->d_i.rs = (IF_ID->d_i.instruction & 0x03e00000) >> 21;
	IF_ID->d_i.rt = (IF_ID->d_i.instruction & 0x001f0000) >> 16;
	IF_ID->d_i.rd = (IF_ID->d_i.instruction & 0x0000f800) >> 11;
	IF_ID->d_i.immediate = IF_ID->d_i.instruction & 0xffff;
	IF_ID->d_i.funct = IF_ID->d_i.instruction & 0x3f;
	IF_ID->d_i.shamt = (IF_ID->d_i.instruction & 0x7c0) >> 6;
	ctrl(&(IF_ID->d_i), &(IF_ID->c_s), p_n);
	ALU_ctrl_sig(&(IF_ID->c_s), IF_ID->d_i.opcode, IF_ID->d_i.funct, &(IF_ID->A_c));
	if((IF_ID->d_i.opcode == 0xc) || (IF_ID->d_i.opcode == 0xd) || (IF_ID->d_i.opcode == 0xf));
	else{
		if((IF_ID->d_i.immediate) & 0x8000){
		IF_ID->d_i.immediate = IF_ID->d_i.immediate | 0xffff0000;
		}
	}
	if(IF_ID->d_i.opcode == 0x3){
		Reg_val(IF_ID->d_i.rs, IF_ID->d_i.rt, 31, &(IF_ID->R_v));
	}
	else{
		Reg_val(IF_ID->d_i.rs, IF_ID->d_i.rt, MUX(IF_ID->d_i.rt, IF_ID->d_i.rd, IF_ID->c_s.RegDst), &(IF_ID->R_v));
	}
	print_stage(IF_ID, 2);
}

void execution(Latch* ID_EX, print_num* p_n){
	if(ctrl_dep == 1){
		bubble_generator(ID_EX);
		print_stage(ID_EX, 3);
		Forwarding_unit_in(ID_EX, 0);
		ctrl_dep = 0;
		return ;
	}
	else if(ctrl_dep == 2){
		bubble_generator(ID_EX);
		print_stage(ID_EX, 3);
		Forwarding_unit_in(ID_EX, 0);
		ctrl_dep = 0;
		return ;
	}
	ID_EX->R_v.Read_data1 = F_mux(ID_EX->R_v.Read_register1, ID_EX->R_v.Read_data1);
	ID_EX->R_v.Read_data2 = F_mux(ID_EX->R_v.Read_register2, ID_EX->R_v.Read_data2);
	if((ID_EX->d_i.opcode == 0xc) || (ID_EX->d_i.opcode == 0xd)){
		ALU(ID_EX->R_v.Read_data1, MUX(ID_EX->R_v.Read_data2, ID_EX->d_i.immediate | 0x00000000, ID_EX->c_s.ALUSrc), ID_EX->A_c.alu_ctrl_num, &(ID_EX->A_v), p_n);
		Forwarding_unit_in(ID_EX, 0);
		print_stage(ID_EX, 3);
		return ;
	}
	else if((ID_EX->d_i.opcode == 0x0) && ((ID_EX->d_i.funct == 0x0) || (ID_EX->d_i.funct == 0x2) || (ID_EX->d_i.funct == 0x3))){
		ALU(ID_EX->R_v.Read_data2, ID_EX->d_i.shamt, ID_EX->A_c.alu_ctrl_num, &(ID_EX->A_v), p_n);
		Forwarding_unit_in(ID_EX, 0);
		print_stage(ID_EX, 3);
		return ;
	}
	else{
		ALU(ID_EX->R_v.Read_data1, MUX(ID_EX->R_v.Read_data2, ID_EX->d_i.immediate, ID_EX->c_s.ALUSrc), ID_EX->A_c.alu_ctrl_num, &(ID_EX->A_v), p_n);
		Forwarding_unit_in(ID_EX, 0);
		if(ID_EX->c_s.Jump){
			if(jump_slot_config(ID_EX->pc_val)){
				ID_EX->c_s.Jump = 0;
			}
			else{
				if(ID_EX->c_s.RegDst) ctrl_dep = 1;
				else{
					ctrl_dep = 1;
					jump_pc_slot_in(ID_EX->pc_val, ID_EX->d_i.address);
				}
			}
		}
		if(branch_flag_config(ID_EX->pc_val) < 2){
			if(ID_EX->c_s.Branch && ID_EX->A_v.ALU_result){
				ctrl_dep = 1;
				branch_slot_in(ID_EX->pc_val, ((ID_EX->d_i.immediate << 2) + ID_EX->pc_val + 4), 1);
				p_n->branch_predict_fail++;
				print_stage(ID_EX, 3);
				return ;
			}
			else{
				if(ID_EX->c_s.Branch){ 
					p_n->branch_predict_success++;
					branch_slot_in(ID_EX->pc_val, ((ID_EX->d_i.immediate << 2) + ID_EX->pc_val + 4), 0);
					print_stage(ID_EX, 3);
					return ;
				}
			}
		}
		else{
			if(ID_EX->c_s.Branch && ID_EX->A_v.ALU_result){
				p_n->branch_predict_success++;
				branch_slot_in(ID_EX->pc_val, ((ID_EX->d_i.immediate << 2) + ID_EX->pc_val + 4), 1);

				ID_EX->c_s.Branch = 0;
				print_stage(ID_EX, 3);
				return ;
			}
			else{
				ctrl_dep = 2;
				npc = ID_EX->pc_val + 4;
				p_n->branch_predict_fail++;
				branch_slot_in(ID_EX->pc_val, ((ID_EX->d_i.immediate << 2) + ID_EX->pc_val + 4), 0);
				print_stage(ID_EX, 3);
				return ;
			}
		}
		print_stage(ID_EX, 3);
		return ;
	}
}

void MEM_Access(Latch* EX_MEM, print_num* p_n){
	EX_MEM->D_M.Address = EX_MEM->A_v.ALU_result;
	EX_MEM->D_M.Write_data = EX_MEM->R_v.Read_data2;
	if(EX_MEM->c_s.MemWrite || EX_MEM->c_s.MemRead){
		p_n->MEM_Access_num++;
		if(EX_MEM->c_s.MemWrite){
			EX_MEM->D_M.Read_data = 0;
			switch(EX_MEM->d_i.opcode){
				case 0x28:
					memory[(EX_MEM->D_M.Address/4)-1] = EX_MEM->D_M.Write_data & 0x000000ff;
					break ;
				case 0x29:
					memory[(EX_MEM->D_M.Address/4)-1] = EX_MEM->D_M.Write_data & 0x0000ffff;
					break ;
				case 0x2b:
				case 0x38:
					memory[(EX_MEM->D_M.Address/4)-1] = EX_MEM->D_M.Write_data;
					break ;
				default:
					break;
			}
		}
		else if(EX_MEM->c_s.MemRead){
			switch(EX_MEM->d_i.opcode){
				case 0x24:
					EX_MEM->D_M.Read_data = memory[(EX_MEM->D_M.Address/4)-1] & 0x000000ff;
					Forwarding_unit_in(EX_MEM, 1);
					break ;
				case 0x25:
					EX_MEM->D_M.Read_data = memory[(EX_MEM->D_M.Address/4)-1] & 0x0000ffff;
					Forwarding_unit_in(EX_MEM, 1);
					break ;
				case 0x23:
				case 0x30:
					EX_MEM->D_M.Read_data = memory[(EX_MEM->D_M.Address/4)-1];
					Forwarding_unit_in(EX_MEM, 1);
					break ;
				default:
					break;
			}
		}
		else ;
		print_stage(EX_MEM, 4);
	}
	else{
		EX_MEM->D_M.Read_data = 0;
		print_stage(EX_MEM, 4);
		return ;
	}
}

void Write_Back(Latch* MEM_WB){
	MEM_WB->R_v.Write_data = MUX(MEM_WB->A_v.ALU_result, MEM_WB->D_M.Read_data,  MEM_WB->c_s.MemtoReg);
	if(MEM_WB->c_s.RegWrite){
		if(MEM_WB->d_i.opcode == 0x3){
			reg[31] = MEM_WB->pc_val+8;
		}
		else if((MEM_WB->d_i.opcode == 0x0) && (MEM_WB->d_i.funct == 0x9)){
			reg[MEM_WB->R_v.Write_register] = MEM_WB->pc_val+4;
		}
		else{
			reg[MEM_WB->R_v.Write_register] = MEM_WB->R_v.Write_data;
		}
	}
	print_stage(MEM_WB, 5);
}

void print_cycle_end(print_num p_n){
	printf("--------------------end--------------------\n");
	printf("executed cycle number: %d\n", p_n.cycle_num-1);
	printf("executed R_type number: %d\n", p_n.R_type_num);
	printf("executed I_type number: %d\n", p_n.I_type_num);
	printf("executed J_type number: %d\n", p_n.J_type_num);
	printf("Memory Access number: %d\n", p_n.MEM_Access_num);
	printf("branch number: %d\n", p_n.branch_num);
	printf("branch satisfied: %d\n", p_n.branch_satisfied_num);
	printf("branch predict success: %d\n", p_n.branch_predict_success);
	printf("branch predict fail: %d\n", p_n.branch_predict_fail);
	printf("result = %d\n", reg[2]);
}

void print_cycle(){
	printf("----------cycle end----------\n");
	printf("v0 = %d\n", reg[2]);
	printf("v1 = %d\n", reg[3]);
	printf("sp = 0x%x\n", reg[29]);
	printf("s8 = 0x%x\n", reg[30]);
	printf("ra = 0x%x\n", reg[31]);
	printf("\n");
}

void print_stage(Latch* L, int i){
	switch(i){
		case 1:
			printf("----------IF----------\n");
			printf("pc = 0x%x\n", L->pc_val);
			printf("instruction = 0x%x\n", L->d_i.instruction);
			break;
		case 2:
			printf("----------ID----------\n");
			printf("pc = 0x%x\n", L->pc_val);
			printf("instruction = 0x%x\n", L->d_i.instruction);
			printf("opcode = 0x%x\n", L->d_i.opcode);
			printf("funct = 0x%x\n", L->d_i.funct);
			printf("rs = %d\n", L->d_i.rs);
			printf("rt = %d\n", L->d_i.rt);
			printf("rd = %d\n", L->d_i.rd);
			printf("imm = %d\n", L->d_i.immediate);
			printf("shamt = %d\n", L->d_i.shamt);
			break;
		case 3:
			printf("----------EX----------\n");
			printf("pc = 0x%x\n", L->pc_val);
			printf("instruction = 0x%x\n", L->d_i.instruction);
			printf("Read_data1 = %d\n", L->R_v.Read_data1);
			printf("Read_data2 = %d\n", L->R_v.Read_data2);
			printf("immediate = %d\n", L->d_i.immediate);
			printf("ALU_result = %d\n", L->A_v.ALU_result);
			break;
		case 4:
			printf("----------MEM----------\n");
			printf("pc = 0x%x\n", L->pc_val);
			printf("instruction = 0x%x\n", L->d_i.instruction);
			printf("Write_data = %d\n", L->D_M.Write_data);
			printf("Read_data = %d\n", L->D_M.Read_data);
			break;
		case 5:
			printf("----------WB----------\n");
			printf("pc = 0x%x\n", L->pc_val);
			printf("instruction = 0x%x\n", L->d_i.instruction);
			printf("Write_data = %d\n", L->R_v.Write_data);
			break;
		defult:
			break;
		}
}
