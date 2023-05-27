#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<limits.h>
#include<errno.h>


// some reg. name aliases

#define zero    0
#define at      1
#define v0      2
#define v1      3
#define a0      4
#define a1      5
#define a2      6
#define a3      7
#define t0      8
#define t1      9
#define t2      10
#define t3      11
#define t4      12
#define t5      13
#define t6      14
#define t7      15
#define s0      16
#define s1      17
#define s2      18
#define s3      19
#define s4      20
#define s5      21
#define s6      22
#define s7      23
#define t8      24
#define t9      25
#define k0      26
#define k1      27
#define gp      28
#define sp      29
#define fp      30
#define ra      31

enum inst_type {INV, R, I, J};

struct inst_t{
    unsigned int opcode;
    unsigned int rs, rd, rt;
    unsigned int shamt, func;
    unsigned int imm;
    unsigned int imm_j;
    int          pc_changed_flag;
};
int instruction_reg;
// global instruction register
static int global_clock;

// some definition
#define MAX_REG 32
#define MAX_MEM 16*1024*1024/sizeof(int)

// global register file
unsigned int regfile[MAX_REG];
// instruction pointer
unsigned int PC;
// memory
unsigned int Mem[MAX_MEM];

void initialize_machine();
void load_program(FILE *fd);
int fetch_instruction(FILE *fd);
int decode_instruction(struct inst_t *inst);
int execute_instruction(struct inst_t *inst);
int load_store(struct inst_t *inst);
int write_back(struct inst_t *inst);
void clock();
void print_instruction(int opcode, int rs, int rt, int rd, int shamt, int func, int imm, int imm_j);

int main(int argc, char* argv[]){
    FILE *fd;
    struct inst_t instruction;
    int ret;

    char *filename;
    if (argc == 2){
        filename = argv[1];
    } else {
        filename = "input.bin";
    }

    // Load the program on the Memory from the file!
    fd = fopen(filename, "r");
    if (fd == NULL){
        exit(1);
    }
    load_program(fd);
    fclose(fd);

    // Ready to go!
    initialize_machine();


     // Fire! CPU begins from initial memory starting point
    do{
        clock();
        ret = fetch_instruction(fd);
        if (ret <= 0) break; // EOR or error
        ret = decode_instruction(&instruction);
        if (ret <= 0) break; // ''
        ret = execute_instruction(&instruction);
        if (ret <= 0) break; // ''
        ret = load_store(&instruction);
        if (ret <= 0) break;
        ret = write_back(&instruction);
        if (ret <= 0) break;
    } while (ret > 0);

    return 0;
}

void initialize_machine(){
    bzero(regfile, sizeof(regfile));
    PC = 0;
    regfile[sp] = 0x1000000;
    regfile[ra] = 0xFFFFFFFF;
}

void load_program(FILE *fd){
    size_t ret = 0;
    int mips_val, i = 0;
    int mem_val;
    do{
        mips_val = 0;
        ret = fread(&mips_val, 1, 4, fd);
        mem_val = ntohl(mips_val);
        Mem[i] = mem_val;
        printf("(%d) load Mem[%X] pa: 0x%X val: 0x%08X\n", (int)ret, i*4, &Mem[i], Mem[i]);
        i++;
    } while (ret == 4);
}

int fetch_instruction(FILE *fd){
    ssize_t ret = 1;
    size_t len = 0;

    if (PC == 0xFFFFFFFF) return 0;
    memset(&instruction_reg, 0, sizeof(instruction_reg));
    instruction_reg = Mem[PC/4];
    printf("Fet[%08X]: %08X\n", PC, instruction_reg);
    return ret;
}

int decode_instruction(struct inst_t *inst)
{
    int ret = 1;
    unsigned int opcode;
    unsigned int rs, rd, rt;
    unsigned int imm, imm_j;
    unsigned int shamt, func;

    opcode = (instruction_reg & 0xFC000000);
    opcode = opcode >> 26;

    rs = (instruction_reg & 0x03E00000);
    rs = rs >> 21;
    rt = (instruction_reg & 0x001F0000);
    rt = rt >> 16;
    rd = (instruction_reg & 0x0000F800);
    rd = rd >> 11;
    shamt = (instruction_reg & 0x000007C0);
    shamt = shamt >> 5;
    func = (instruction_reg & 0x0000003F);
    imm = (instruction_reg & 0x0000FFFF);
    imm_j = (instruction_reg & 0x03FFFFFF);

    bzero(inst, sizeof(*inst));
    inst->opcode = opcode;
    inst->rs = rs;
    inst->rt = rt;
    inst->rd = rd;
    inst->shamt = shamt;
    inst->func = func;
    inst->imm = imm;
    inst->imm_j = imm_j;

    printf("Dec: ");
    print_instruction(opcode, rs, rt, rd, shamt, func, imm, imm_j);

    return ret;
}

void print_instruction(int opcode, int rs, int rt, int rd, int shamt, int func, int imm, int imm_j){
    printf("opcode: %x, rs: %x, rt: %x, shamt: %x, func: %x, imm: %x, imm_j: %x\n", opcode, rs, rt, rd, shamt, func, imm, imm_j);
}

int execute_instruction(struct inst_t *i){
    unsigned int opcode, rs, rd, rt, shamt, func, imm, imm_j;
    int ret = 1;
    int s_bit;
    int s_ext_imm;
    int z_ext_imm;
    int *b_addr;
    unsigned int *j_addr;

    unsigned int utemp;
    unsigned int utemp2;
    int temp;
    int pc_changed_flag = 0;

    opcode = i->opcode;
    rs = i->rs;
    rt = i->rt;
    rd = i->rd;
    shamt = i->shamt;
    func = i->func;
    imm = i->imm;
    imm_j = i->imm_j;

    // check sign-bit of imm
    s_bit = imm >> 15; // 0 for positive, 1 for negative
    if (s_bit){
        short s_imm = (short)imm;
        s_ext_imm = s_imm;
    } else{
        s_ext_imm = imm;
    }
    z_ext_imm = imm;

    utemp = imm << 2;
    b_addr = imm << 2;
    utemp = (unsigned int)PC + 4;
    utemp = utemp & 0xF0000000;
    utemp2 = imm_j << 2;
    utemp = utemp | utemp2;
    j_addr = (unsigned int *)utemp;

    printf("Exe: ");
    // main execution loop
    switch(opcode)
    {
    case 0:
        switch(func){
        case 0x20:
            regfile[rd] = (int)((int)regfile[rs] + (int)regfile[rt]);
        break;
        case 0x21:
            regfile[rd] = regfile[rs] + regfile[rt];
        break;
        case 0x24:
            regfile[rd] = regfile[rs] & regfile[rt];
        break;
        case 0x8:
            utemp = regfile[rs];
            PC = utemp;
            pc_changed_flag = 1;
        break;
        case 0x27:
            regfile[rd] = ~(regfile[rs]|regfile[rt]);
        break;
        case 0x25:
            regfile[rd] = regfile[rs]|regfile[rt];
        break;
        case 0x2a:
            regfile[rd] = ((int)regfile[rs] < (int)regfile[rt]) ? 1:0;
        break;
        case 0x2b:
            regfile[rd] = (regfile[rs] < regfile[rt]) ? 1:0;
        break;
        case 0x0:
            regfile[rd] = regfile[rt] << shamt;
        break;
        case 0x2:
            regfile[rd] = regfile[rt] >> shamt;
        break;
        case 0x22:
            regfile[rd] = (int)((int)regfile[rs] - (int)regfile[rt]);
        break;
        case 0x23:
            regfile[rd] = regfile[rs] - regfile[rt];
        break;
        }
    break;
    case 0x8:
        regfile[rt] = (int)((int)regfile[rs]+s_ext_imm);
    break;
    case 0x9:
        regfile[rt] = (unsigned int)regfile[rs]+s_ext_imm;
    break;
    case 0xc:
        regfile[rt] = regfile[rs] & z_ext_imm;
    break;
    case 0x4:
        if(regfile[rs] == regfile[rt]){
            utemp = PC+4+(short)b_addr;
            PC = utemp;
            pc_changed_flag = 1;
        }
    break;
    case 0x5:
        if(regfile[rs] != regfile[rt]){
            utemp = PC+4+(short)b_addr;
            PC = utemp;
            pc_changed_flag = 1;
        }
    break;
    case 0x2:
        utemp = (unsigned int)j_addr;
        PC = utemp;
        pc_changed_flag = 1;
    break;
    case 0x3:
        regfile[ra] = PC+8;
        utemp = (unsigned int)j_addr;
        PC = utemp;
        pc_changed_flag = 1;
    break;
    case 0x24:
        utemp2 = (unsigned int)s_ext_imm & 0x000000FF;
        utemp = regfile[rs]+utemp2;
        utemp = Mem[utemp/sizeof(unsigned int)];
        regfile[rt] = utemp & 0xFF;
    break;
    case 0x25:
        utemp2 = (unsigned int)s_ext_imm & 0x000000FF;
        utemp = regfile[rs]+utemp2;
        utemp = Mem[utemp/sizeof(unsigned int)];
        regfile[rt] = utemp & 0xFFFF;
    break;
    case 0x30:
        utemp2 = (unsigned int)s_ext_imm & 0x000000FF;
        utemp = regfile[rs]+utemp2;
        utemp = Mem[utemp/sizeof(unsigned int)];
        regfile[rt] = utemp;
    break;
    case 0xf:
        utemp = imm << 16;
        regfile[rt] = utemp;
    break;
    case 0x23:
        utemp2 = (unsigned int)s_ext_imm & 0x000000FF;
        utemp = regfile[rs]+utemp2;
        utemp = Mem[utemp/sizeof(unsigned int)];
        regfile[rt] = utemp;
    break;
    case 0xd:
        utemp = regfile[rs] | z_ext_imm;
        regfile[rt] = utemp;
    break;
    case 0xa:
        regfile[rt] = (regfile[rs] < s_ext_imm) ? 1:0;
    break;
    case 0xb:
        regfile[rt] = (regfile[rs] < (unsigned int)s_ext_imm) ? 1:0;
    break;
    case 0x28:
        utemp2 = (unsigned int)s_ext_imm;
        utemp = regfile[rs]+utemp2;
        utemp2 = Mem[utemp/sizeof(unsigned int)];
        utemp2 = utemp2 & 0xFFFFFF00;
        utemp2 = (regfile[rt] & 0xFF) | utemp2;
        Mem[utemp/sizeof(unsigned int)] = utemp2;
    break;
    case 0x38:
        utemp2 = (unsigned int)s_ext_imm;
        utemp = regfile[rs]+utemp2;
        utemp2 = regfile[rt];
        Mem[utemp/sizeof(unsigned int)] = utemp2;
        regfile[rt] = 1; // atomic
    break;
    case 0x29:
        utemp2 = (unsigned int)s_ext_imm;
        utemp = regfile[rs]+utemp2;
        utemp2 = Mem[utemp/sizeof(unsigned int)];
        utemp2 - utemp2 & 0xFFFF0000;
        utemp2 = (regfile[rt] & 0xFFFF) | utemp2;
        Mem[(utemp>>2)] = utemp2;
    break;
    case 0x2b:
        utemp2 = (unsigned int)s_ext_imm;
        utemp = regfile[rs]+utemp2;
        utemp2 = regfile[rt];
        Mem[(utemp>>2)] = utemp2;
    break;
    default:
    ret = -1; // INVALID OPCODE
    break;
    }
    printf("\n");
    i->pc_changed_flag = pc_changed_flag;
    return ret;
}

int load_store(struct inst_t *i){
    int ret = 1;
    return ret;
}

int write_back(struct inst_t *i){
    unsigned int opcode, rs, rt, rd, shamt, func, imm, imm_j;

    int ret = 1;
    int s_bit;
    int s_ext_imm;
    int z_ext_imm;
    int *b_addr;
    unsigned int *j_addr;

    unsigned int utemp;
    unsigned int utemp2;
    int temp;
    int pc_changed_flag = 0;

    printf("WB: ");
    opcode = i->opcode;
    rs = i->rs;
    rd = i->rd;
    rt = i->rt;
    shamt = i->shamt;
    func = i->func;
    imm = i->imm;
    imm_j = i->imm_j;

    // check sign-bit of imm
    s_bit = imm >> 15; // 0 for positive, 1 for negative
    if (s_bit){
        short s_imm = (short)imm;
        s_ext_imm = s_imm;
    } else{
        s_ext_imm = imm;
    }
    z_ext_imm = imm;

    utemp = imm << 2;
    b_addr = imm << 2;
    utemp = (unsigned int)PC + 4;
    utemp = utemp & 0xF0000000;
    utemp2 = imm_j << 2;
    utemp = utemp | utemp2;
    j_addr = (unsigned int *)utemp;

    switch(opcode){
    case 0:
        switch(func){
            case 0x20:
                regfile[rd] = (int)((int)regfile[rs] + (int)regfile[rt]);
                printf("ADD\t R%d (0x%x) = R%d + R%d\n", rd, regfile[rd], rs, rt);
            break;
            case 0x21:
                regfile[rd] = (int)((int)regfile[rs] + (int)regfile[rt]);
                printf("ADD\t R%d (0x%x) = R%d + R%d\n", rd, regfile[rd], rs, rt);
            break;
            case 0x24:
                regfile[rd] = (int)((int)regfile[rs] + (int)regfile[rt]);
                printf("ADD\t R%d (0x%x) = R%d + R%d\n", rd, regfile[rd], rs, rt);
            break;
            case 0x8:
                utemp = regfile[rs];
                PC = (unsigned int *)utemp;
                pc_changed_flag = 1;
                printf("JR\t PC = R%d (0x%x)\n", rs, regfile[rs]);
            break;
            case 0x9:
                utemp = regfile[rs];
                PC = (unsigned int *)utemp;
                pc_changed_flag = 1;
                printf("JALR\t JAL + PC = R%d (0x%x)\n", rs, regfile[rs]);
            break;
            case 0x27:
                regfile[rd] = ~(regfile[rs]|regfile[rt]);
                printf("NOR\t R%d (0x%x) = R%d & R%d\n", rd, regfile[rd], rs, rt);
            break;
            case 0x25:
                regfile[rd] = ~(regfile[rs]|regfile[rt]);
                printf("OR\t R%d (0x%x) = R%d & R%d\n", rd, regfile[rd], rs, rt);
            break;
            case 0x2a:
                regfile[rd] = ((int)regfile[rs] < (int)regfile[rt]) ? 1:0;
                printf("SLT\t R%d = R%d (0x%x) < R%d (0x%x)\n", rd, rs, regfile[rs], rt, regfile[rt]);
            break;
            case 0x2b:
                regfile[rd] = (regfile[rs] < regfile[rt]) ? 1:0;
                printf("SLTU\t R%d = R%d (0x%x) < R%d (0x%x)\n", rd, rs, regfile[rs], rt, regfile[rt]);
            break;
            case 0x0:
                regfile[rd] = regfile[rt] << shamt;
                printf("SLL\t R%d (0x%x) = R%d (0x%x) << %d\n", rd, regfile[rd], rt, regfile[rt], shamt);
            break;
            case 0x2:
                regfile[rd] = regfile[rt] >> shamt;
                printf("SLR\t R%d (0x%x) = R%d (0x%x) >> %d\n", rd, regfile[rd], rt, regfile[rt], shamt);
            break;
            case 0x22:
                regfile[rd] = (int)((int)regfile[rs] - (int)regfile[rt]);
                printf("SUB\t R%d (0x%x) = R%d - R%d\n", rd, regfile[rd], rs, rt);
            break;
            case 0x23:
                regfile[rd] = regfile[rs] - regfile[rt];
                printf("SUBU\t R%d (0x%x) = R%d - R%d\n", rd, regfile[rd], rs, rt);
            break;
            }
        break;
        case 0x8:
            regfile[rt] = (int)((int)regfile[rs]+s_ext_imm);
            printf("ADDI\t R%d (0x%x) = R%d + (%d)\n", rd, regfile[rd], rs, s_ext_imm);
        break;
        case 0x9:
            regfile[rt] = (unsigned int)regfile[rs]+s_ext_imm;
            printf("ADDIU\t R%d (0x%x) = R%d + (%d)\n", rd, regfile[rd], rs, s_ext_imm);
        break;
        case 0xc:
            regfile[rt] = regfile[rs] & z_ext_imm;
            printf("ANDI\t R%d (0x%x) = R%d & (%d)\n", rd, regfile[rd], rs, z_ext_imm);
        break;
        case 0x4:
            if (regfile[rs] == regfile[rt]) {
                utemp = (unsigned int)PC+4+(short)b_addr;
                PC = (unsigned int *)utemp;
                pc_changed_flag = 1;
            }
            printf("BEQ\t b_addr 0x%x if R%d (0x%x) == R%d (0x%x)\n", b_addr, rs, regfile[rs], rt, regfile[rt]);
        break;
        case 0x5:
            if (regfile[rs] != regfile[rt]) {
                utemp = (unsigned int)PC+4+(short)b_addr;
                PC = (unsigned int *)utemp;
                pc_changed_flag = 1;
            }
            printf("BNE\t b_addr 0x%x if R%d (0x%x) != R%d (0x%x)\n", b_addr, rs, regfile[rs], rt, regfile[rt]);
        break;
        case 0x2:
            utemp = (unsigned int)j_addr;
            PC = &Mem[(utemp>>2)];
            pc_changed_flag = 1;
            printf("J\t j_addr 0x%x \n", j_addr);
        break;
        
        case 03:
            regfile[ra]= (unsigned int)PC+8;
            utemp= regfile[rs]+utemp2;
            utemp= Mem[utemp/sizeof(unsigned int)];
            regfile[rt]= utemp & 0xFFFF;
             
            printf("LBU\t R%d(0x%x) MEm[0x%x]\n", rt, regfile[ret], s_ext_imm);
        break;
            
        case 0x30:
            utemp2 =(unsigned int)s_ext_imm;
            utemp =regfile[rs]+utemp2;
            utemp= Mem[utemp/sizeof(unsigned int)];
            regfile[rt]= utemp;
        break;
            
        case 0xf:
            temp= imm<<16;
            regfile[rt]= utemp;
            printf("LW\t R%d (0x%x) Mem[R%d (0x%x)+ 0x%x]\n)", rt, regfile[rt], rs, regfile[rs], s_ext_imm);
        break;
        case 0xd:
            utemp= regfile[rs] | z_ext_imm;
            utemp =utemp;
            printf("ORI\t R%d (0x%x)= R%d | R%d\n", rt, regfile[rt], rs, z_ext_imm);
        break;
        
        case 0xb:
            regfile[rt] =(regfile[rs]< s_ext_imm) ? 1:0;
            printf ("SLTIU\t R%d (%d)<%d\n",rs, regfile[rs], s_ext_imm);
            break;
            
        case 0x28:
            utemp2= (unsigned int)s_ext_imm;
            utemp= regfile[rs]+ utemp2;
            utemp2= Mem[utemp/ sizeof(unsigned int)];
            utemp2= utemp2 & 0xFFFFFF00;
            utemp2= (regfile[rt] & 0xFF)|utemp2;
            Mem[utemp/sizeof (unsigned int)]= utemp2;
            printf("SB\t R%d (0x%x) Mem[0x%x]\n", rt, regfile[rt], s_ext_imm);
        break;
        
        case 0x38:
            utemp2= (unsigned int)s_ext_imm;
            utemp= regfile[rs]+utemp2;
            utemp2= Mem[utemp/ sizeof(unsigned int)];
            utemp2= utemp2 &0xFFFF0000;
            Mem[utemp/ sizeof(unsigned int)]= utemp2;
            printf("SH\t R%d (0x%x) Mem[0x%x]\n", rt, regfile[rt], s_ext_imm);
        break;
            
        case 0x29:
            utemp2= (unsigned int )s_ext_imm;
            utemp= regfile[rs]+utemp2;
            utemp2= Mem[utemp/ sizeof(unsigned int)];
            utemp2= utemp2= utemp2 & 0xFFFF0000;
            utemp2 =(regfile[rt] & 0xFFFF)| utemp2;
            Mem[utemp/ sizeof(unsigned int )]= utemp2;
            printf("SH\t R%d (0x%x) Mem [R%d (0x%x)+ 0x%x]\n", rt, regfile[rt], rs, regfile[rs], s_ext_imm);
        break;
            
            
        case 0x2b:
            utemp2 =(unsigned int)s_ext_imm;
            utemp= regfile[rs]+utemp2;
            utemp2= regfile[rt];
            Mem[utemp/ sizeof(unsigned int)]= utemp2;
            printf("SW\t R%d (0x%x) Mem [R%d (0x%x)+ 0x%x]\n",rt , regfile[rt], rs, regfile[rs], s_ext_imm);
        break;
        }
        if (i -> pc_changed_flag ==0)
            PC= PC+4;
        return ret;
}
void clock()
{   
}
