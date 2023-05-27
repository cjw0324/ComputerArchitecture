#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define Max_reg 32
unsigned int opcode, rs, rt, rd, shamt, funct, imm, imm_j, Change;
unsigned int R[Max_reg];
unsigned int PC;
unsigned int Mem[0x40000];
int instruction_reg;

int fetch(FILE *fp);
int decode();
int execution();
int lwsw();
int writeback();

int main (){
    FILE *fp = fopen("input.bin", "rb");                                  
    int i=0;
    int ret;
    int mem_val, mips_val;

    do{
        mips_val = 0;
        ret = fread(&mips_val, 1, 4, fp);
        mem_val = ntohl(mips_val);
        Mem[i] = mem_val;
        printf("(%d) load Mem[%X] pa: 0x%X val: 0x%08X\n", (int)ret, i*4, &Mem[i], Mem[i]);
        i++;
        PC+=4;
     
    } while(ret == 4);
    
    fclose(fp);
    bzero(R, sizeof(R));
    PC = 0;
    R[29] = 0x1000000;

    R[31] = 0xFFFFFFFF;
    /*int j=0;
  while(j<10){
        ret = fetch(fp);
        if (ret <= 0) break; 
        ret = decode();
        if (ret <= 0) 
            break; 
        j++;
        PC+=4;
       
    }*/ //fetch와 decode 실행확인
    
   do{
        ret = fetch(fp);
        if (ret <= 0) 
            break; 
        ret = decode();
        if (ret <= 0) 
            break; 
       ret = execution();
        if (ret <= 0) 
            break;
        ret = lwsw();
        if (ret <= 0) 
            break;
        ret = writeback();
        if (ret <= 0) 
            break; 

    } while (ret > 0);   


    printf("%d", R[2]); 

    return 0;

}
int fetch(FILE *fp){
    ssize_t ret = 1;

    if (PC == 0xFFFFFFFF) {
        return 0;}

    memset(&instruction_reg, 0, sizeof(instruction_reg));

    instruction_reg = Mem[PC/4];

    printf("PC %08X: %08X\n", PC, instruction_reg);


    return ret;
}

int decode(){
    
   int ret = 1;

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
    funct = (instruction_reg & 0x0000003F);
    imm = (instruction_reg & 0x0000FFFF);
    imm_j = (instruction_reg & 0x03FFFFFF);

    printf("Dec: ");
    printf("opcode: %x, rs: %x, rt: %x, shamt: %x, funct: %x, imm: %x, imm_j: %x\n", opcode, rs, rt, rd, shamt, funct, imm, imm_j);
    printf("%x",instruction_reg);
    return ret;

}
int execution(){
    int ret=1;
    int sign;
    int zero, branch,temp,temp2; 
    Change= 0;
    
    if ((imm>>15)==1){
            sign=(0xffff0000|imm);
    }
    else{
        sign=imm;
        }
    zero= imm;
 
    branch=(sign << 2);

    switch(opcode){
        //R Type
        case(0x0):
            switch(funct){
                case(0x20):
                    R[rd]=R[rs]+R[rt];
                    break;
                case(0x21):
                    R[rd]=R[rs]+R[rt];
                    break;
                case(0x24):
                    R[rd]=(R[rs]&R[rt]);
                    break;
                case(0x08):
                    PC= R[rs];
                    break;
                case(0x27):
                    R[rd]= ~(R[rs]|R[rt]);
                    break;
                case(0x25):
                    R[rd]= R[rs]|R[rt];
                    break;
                case(0x2a):
                    R[rd]=(R[rs]<R[rt])? 1:0;
                    break;
                case(0x2b):
                    R[rd]=(R[rs]<R[rt])? 1:0;
                    break;
                case(0x00):
                    R[rd]=R[rt]<<shamt;
                    break;
                case(0x02):
                    R[rd]=R[rt]>>shamt;
                    break;
                case(0x22):
                    R[rd]=R[rs]-R[rt];
                    break;
                case(0x23):
                    R[rd]=R[rs]-R[rt];
                    break;
            }
        
        //I Type
        case(0x8):
           R[rt]=R[rs]+sign;
           break;
        case(0x9):
            R[rt]=R[rs]+sign;
            break;
        case(0xc):
            R[rt]=R[rs]+zero;
            break;
        case(0x4):
            if(R[rs]==R[rt]){
                PC=PC+4+branch;
                Change=1;
                break;
            }
        case(0x5):
        if(R[rs]==R[rt]){
              PC=PC+4+branch;
              Change=1; 
              break;}
        case(0x24):
            temp = R[rs]+(unsigned int)sign & 0x000000FF;
            R[rt] = (Mem[temp/sizeof(unsigned int)]) & 0x000000FF;
            break;
        case(0x25):
            temp = R[rs]+(unsigned int)sign & 0x0000FFFF;
            R[rt] = (Mem[temp/sizeof(unsigned int)]) & 0x0000FFFF;
            break; 
        case(0x30):
            R[rt]= Mem[R[rs]+sign/sizeof(unsigned int)];
            break;
        case(0xf):
            R[rt]=imm<<16;
            break;
        case(0x23):
            temp2 = (unsigned int)sign & 0x000000FF;
            temp = R[rs]+temp2;
            temp = Mem[temp/sizeof(unsigned int)];
            R[rt] = temp;
            break;

        case(0xd):
            R[rt]=R[rs]|zero;
            break;
        case(0xa):
            R[rt]=(R[rs]<sign)?1:0;
            break;
        case(0xb):
            R[rt]=(R[rs]<sign)?1:0;
            break;
        case(0x28):
            
            temp2 = (unsigned int)sign;
            temp = R[rs]+temp2;
            temp2 = Mem[temp/sizeof(unsigned int)];
            temp2 = temp2 & 0xFFFFFF00;
            temp2 = (R[rt] & 0xFF) | temp2;
            Mem[temp/sizeof(unsigned int)] = temp2;
        break;
        case(0x38):
            temp = (unsigned int)sign;
            temp2 = R[rs]+temp;
            temp = R[rt];
            Mem[temp2/sizeof(unsigned int)] = temp;
            R[rt] = 1;           
            break; 

        case(0x29):
            temp2 = (unsigned int)sign;
            temp = R[rs]+temp2;
            temp2 = Mem[temp/sizeof(unsigned int)];
            temp2 = temp2 & 0xFFFF0000;
            temp2 = (R[rt] & 0xFFFF) | temp2;
            Mem[(temp>>2)] = temp2;
            break;
        case(0x2b):
            temp2 = (unsigned int)sign;
            temp = R[rs]+temp2;
            temp2 = R[rt];
        Mem[(temp>>2)] = temp2;
        break;


        case(0x2):
            PC = imm_j;
            Change= 1;
            break;
        case(0x3):
            R[31] = (unsigned int)PC+8;
            PC = imm_j;
            Change = 1;
            break; 
    if (opcode!=0){
        ret=0;
    }
         
    }
if(Change==0){
    PC+=4;
}
return ret;
}

int lwsw(){
    int ret=1;
    return ret;
}
int writeback(){
    int ret=1;
    return ret;
}
