#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 1024


int getLine();
void readFile();
long sti(char *str);
int chooseType(char *op);
long calc(int type,long x,long y );
long add(long x, long y);
long sub(long x, long y);
long mul(long x, long y);
long divv(long x, long y);
long bigger(char*op1, char*op2);
long same(char*op1,char*op2, long line_number);

static long R0, R1, R2, R3, R4, R5, R6, R7, R8, R9 = 0;

char buffer[MAX];

int getLine(){
	FILE *file = fopen("input.txt", "r");
    	int line_count = 0;
    	char ch;

    	if (file == NULL) {
        	printf("Failed to open file.\n");
        	return 1;
    	}

    	while (fscanf(file,"%c", &ch) != EOF) {
        	if (ch == '\n') {
            		line_count++;
        	}
    	}

    	fclose(file);
	return line_count;
}

void move(char index, long result)
{
    switch (index)
    {
    case '0':
        R0 = result;
        break;
    case '1':
        R1 = result;
        break;
    case '2':
        R2 = result;
        break;
    case '3':
        R3 = result;
        break;
    case '4':
        R4 = result;
        break;
    case '5':
        R5 = result;
        break;
    case '6':
        R6 = result;
        break;
    case '7':
        R7 = result;
        break;
    case '8':
        R8 = result;
        break;
    case '9':
        R9 = result;
        break;
    default:
        break;
    }
//    printf("R%c : %ld\n", index, result);
}


void readFile(){
	int line = 1;
	int total_line = getLine();
	char op[4][MAX];
	int currentLine = 0;
	FILE *fs;
	fs = fopen("input.txt", "r");
	while (fgets(buffer, MAX, fs) != NULL)
    	{
		currentLine++;
		memset(op,0,sizeof(op));
		//line 값보다 현재 currentLine 이 작다면, 다음 줄로 넘어간다.
        	if(currentLine < line)
		{
			printf("pass %d line\n", currentLine);
			continue;
		}
		else if(currentLine >= line)
		{
	        	char *ptr = strtok(buffer, " ");
	        	int i = 0;
	        	while (ptr != NULL)
	        	{
	            		strcpy(op[i], ptr);
	            		ptr = strtok(NULL, " ");
	            		i++;
	        	}
			int type = chooseType(op[0]);

			printf("execute %d line : %s %s %s %s\n", currentLine,  op[0], op[1], op[2], op[3]);

			if(type == 1 || type == 2 || type == 3 || type == 4){
				if(op[2][1] == 'x'){   //첫번째 피연산자가 16진수 숫자일때
					R1 = strtol(op[2], NULL, 16);
				}
				if(op[2][1] != 'x'){           //첫번째 피연산자가 변수일때
					switch (op[2][1])
	            			{
	            				case '0':
	                				R1 = R0;
							break;
	            				case '1':
	                				R1 = R1;
	                				break;
	            				case '2':
	                				R1 = R2;
	                				break;
	            				case '3':
	                				R1 = R3;
	                				break;
	            				case '4':
	                				R1 = R4;
	                				break;
	            				case '5':
	                				R1 = R5;
	                				break;
	            				case '6':
	                				R1 = R6;
	                				break;
	            				case '7':
	                				R1 = R7;
	                				break;
	            				case '8':
	                				R1 = R8;
	                				break;
	            				case '9':
	                				R1 = R9;
	                				break;
	            				default:
	                				break;
	            			}
				}
				if(op[3][1] == 'x'){ //두번째 피연산자가 16진수 숫자일때
					R2 = strtol(op[3], NULL, 16);
				}
				if(op[3][1] != 'x'){         //두번째 피연산자가 변수일때
					switch (op[3][1])
	            			{
	            				case '0':
	                				R2 = R0;
	               					break;
	            				case '1':
	                				R2 = R1;
	                				break;
	            				case '2':
	                				R2 = R2;
	                				break;
	            				case '3':
	                				R2 = R3;
	                				break;
	            				case '4':
	                				R2 = R4;
	                				break;
	            				case '5':
	                				R2 = R5;
	                				break;
	            				case '6':
	                				R2 = R6;
	                				break;
	            				case '7':
	                				R2 = R7;
	                				break;
	            				case '8':
	                				R2 = R8;
	                				break;
	            				case '9':
	                				R2 = R9;
	                				break;
	            				default:
	                				break;
	            			}
				}
				if(strcmp(op[1],"r0")==0){
					R0 = calc(type, R1,R2);
                                        //printf("R1 = %ld\n", R0);
				}
				else if(strcmp(op[1],"r1")==0){
					R1 = calc(type, R1,R2);
					//printf("R1 = %ld\n", R1);
				}
				else if(strcmp(op[1],"r2")==0){
					R2 = calc(type, R1, R2);
					//printf("R2 = %ld\n", R2);
				}
				else if(strcmp(op[1],"r3")==0){
	                                R3 = calc(type, R1, R2);
					//printf("R3 = %ld\n", R3);
	                        }
				else if(strcmp(op[1],"r4")==0){
	                                R4 = calc(type, R1, R2);
					//printf("R4 = %ld\n", R4);
	                        }
				else if(strcmp(op[1],"r5")==0){
	                                R5 = calc(type, R1, R2);
					//printf("R5 = %ld\n", R5);
	                        }
				else if(strcmp(op[1],"r6")==0){
	                                R6 = calc(type, R1, R2);
					//printf("R6 = %ld\n", R6);
	                        }
				else if(strcmp(op[1],"r7")==0){
	                                R7 = calc(type, R1, R2);
	                                //printf("R7 = %ld\n", R7);
	                        }
				 else if(strcmp(op[1],"r8")==0){
	                                R8 = calc(type, R1, R2);
	                                //printf("R8 = %ld\n", R8);
	                        }
				else if(strcmp(op[1],"r9")==0){
	                                R9 = calc(type, R1, R2);
	                                //printf("R9 = %ld\n", R9);
	                        }
			}
			else if(type == 5){  //move 처리
				switch (op[2][1])
			            {
			            case 'x':
			                R0 = strtol(op[2], NULL, 16);
			                move(op[1][1], R0);
			                break;
			            case '0':
			                move(op[1][1], R0);
			                break;
			            case '1':
			                move(op[1][1], R1);
			                break;
			            case '2':
			                move(op[1][1], R2);
			                break;
			            case '3':
			                move(op[1][1], R3);
			                break;
			            case '4':
			                move(op[1][1], R4);
			                break;
			            case '5':
			                move(op[1][1], R5);
			                break;
			            case '6':
			                move(op[1][1], R6);
			                break;
			            case '7':
			                move(op[1][1], R7);
			                break;
			            case '8':
			                move(op[1][1], R8);
			                break;
			            case '9':
			                move(op[1][1], R9);
			                break;
			            default:
			                break;
			            }
			}
			else if(type == 6){    //LW
				switch(op[1][1])
				{
					case '0':
						R0 = strtol(op[2], NULL, 16);
						break;
					case '1':
						R1 = strtol(op[2], NULL, 16);
						break;
					case '2':
						R2 = strtol(op[2], NULL, 16);
						break;
	                                case '3':
						R3 = strtol(op[2], NULL, 16);
						break;
					case '4':
						R4 = strtol(op[2], NULL, 16);
						break;
	                                case '5':
						R5 = strtol(op[2], NULL, 16);
						break;
					case '6':
						R6 = strtol(op[2], NULL, 16);
						break;
	                                case '7':
						R7 = strtol(op[2], NULL, 16);
						break;
					case '8':
						R8 = strtol(op[2], NULL, 16);
						break;
	                                case '9':
						R9 = strtol(op[2], NULL, 16);
						break;
					default:
						break;
				}
			}
			else if(type == 7){     //SW
				switch(op[1][1])
				{
					case '0':
						printf("R%c : %ld\n",op[1][1] ,R0);
						break;
					case '1':
	                                        printf("R%c : %ld\n",op[1][1] , R1);
						break;
					case '2':
	                                        printf("R%c : %ld\n",op[1][1] , R2);
						break;
					case '3':
	                                        printf("R%c : %ld\n",op[1][1] , R3);
						break;
					case '4':
	                                        printf("R%c : %ld\n",op[1][1] , R4);
						break;
					case '5':
	                                        printf("R%c : %ld\n",op[1][1] , R5);
						break;
					case '6':
	                                        printf("R%c : %ld\n",op[1][1] , R6);
						break;
					case '7':
	                                        printf("R%c : %ld\n",op[1][1] , R7);
						break;
					case '8':
	                                        printf("R%c : %ld",op[1][1] , R8);
						break;
					case '9':
	                                        printf("R%c : %ld",op[1][1] , R9);
						break;
				}
			}
			else if(type == 8){     //RST
				R0=0;
				R1=0;
				R2=0;
				R3=0;
				R4=0;
				R5=0;
				R6=0;
				R7=0;
				R8=0;
				R9=0;
			}
			else if(type == 9){      //JMP
				line = strtol(op[1], NULL, 16);
			}
			else if(type == 10){     //BEQ
				if(same(op[1],op[2], strtol(op[3], NULL, 16))!=0){
					line = strtol(op[3], NULL, 16);
				}
			}
			else if(type == 11){     //BNE
				if(same(op[1],op[2], strtol(op[3], NULL, 16))==0){
                                        line = strtol(op[3], NULL, 16);
                                }
			}
			else if(type == 12){     //SLT
				switch(op[1][1])
                                {
					case '0':
                                                R0 = bigger(op[2],op[3]);
                                                break;
					case '1':
                                                R1 = bigger(op[2],op[3]);
                                                break;
					case '2':
                                                R2 = bigger(op[2],op[3]);
                                                break;
					case '3':
                                                R3 = bigger(op[2],op[3]);
                                                break;
					case '4':
                                                R4 = bigger(op[2],op[3]);
                                                break;
					case '5':
                                                R5 = bigger(op[2],op[3]);
                                                break;
					case '6':
                                                R6 = bigger(op[2],op[3]);
                                                break;
					case '7':
                                                R7 = bigger(op[2],op[3]);
                                                break;
					case '8':
                                                R8 = bigger(op[2],op[3]);
                                                break;

					case '9':
                                                R9 = bigger(op[2],op[3]);
                                                break;
				}
			}
		}

		printf("R0: %ld / R1: %ld / R2: %ld / R3: %ld / R4: %ld / R5: %ld / R6: %ld / R7: %ld / R8: %ld / R9: %ld\n", R0,  R1, R2, R3, R4, R5, R6, R7, R8, R9);
		printf("====================================================================================\n\n");
	}
}


long bigger(char *op1, char *op2){
	long l1, l2;

        if(op1[1] == 'x'){   //첫번째 피연산자가 16진수 숫자일때
                l1 = strtol(op1, NULL, 16);
        }
        if(op1[1] != 'x'){           //첫번째 피연산자가 변수일때
                switch (op1[1])
                {
                        case '0':
                                l1 = R0;
                                break;
                        case '1':
                                l1 = R1;
                                break;
                        case '2':
                                l1 = R2;
                                break;
                        case '3':
                                l1 = R3;
                                break;
                        case '4':
                                l1 = R4;
                                break;
                        case '5':
                                l1 = R5;
                                break;
                        case '6':
                                l1 = R6;
                                break;
                        case '7':
                                l1 = R7;
                                break;
                        case '8':
                                l1 = R8;
                                break;
			case '9':
                                l1 = R9;
                                break;
		}
	}
	if(op2[1] == 'x'){ //두번째 피연산자가 16진수 숫자일때
		l2 = strtol(op2, NULL, 16);
        }
        if(op2[1] != 'x'){         //두번째 피연산자가 변수일때
                switch (op2[1])
                {
                        case '0':
                                l2 = R0;
                                break;
                        case '1':
                                l2 = R1;
                                break;
                        case '2':
                                l2 = R2;
                                break;
                        case '3':
                                l2 = R3;
                                break;
                        case '4':
                                l2 = R4;
                                break;
                        case '5':
                                l2 = R5;
                                break;
                        case '6':
                                l2 = R6;
                                break;
                        case '7':
                                l2 = R7;
                                break;
                        case '8':
                                l2 = R8;
                                break;
                        case '9':
                                l2 = R9;
                                break;
                        default:
                                break;
                }
        }
	if(l1 < l2){
		return 1;
	}
	else{
		return 0;
	}
}

long same(char*op1, char*op2, long line_number){
	long l1, l2;

        if(op1[1] == 'x'){   //첫번째 피연산자가 16진수 숫자일때
                l1 = strtol(op1, NULL, 16);
        }
        if(op1[1] != 'x'){           //첫번째 피연산자가 변수일때
                switch (op1[1])
                {
                        case '0':
                                l1 = R0;
                                break;
                        case '1':
                                l1 = R1;
                                break;
                        case '2':
                                l1 = R2;
                                break;
                        case '3':
                                l1 = R3;
                                break;
                        case '4':
                                l1 = R4;
                                break;
                        case '5':
                                l1 = R5;
                                break;
                        case '6':
                                l1 = R6;
                                break;
                        case '7':
                                l1 = R7;
                                break;
                        case '8':
                                l1 = R8;
                                break;
			case '9':
                                l1 = R9;
                                break;
		}
	}
	if(op2[1] == 'x'){ //두번째 피연산자가 16진수 숫자일때
		l2 = strtol(op2, NULL, 16);
        }
        if(op2[1] != 'x'){         //두번째 피연산자가 변수일때
                switch (op2[1])
                {
                        case '0':
                                l2 = R0;
                                break;
                        case '1':
                                l2 = R1;
                                break;
                        case '2':
                                l2 = R2;
                                break;
                        case '3':
                                l2 = R3;
                                break;
                        case '4':
                                l2 = R4;
                                break;
                        case '5':
                                l2 = R5;
                                break;
                        case '6':
                                l2 = R6;
                                break;
                        case '7':
                                l2 = R7;
                                break;
                        case '8':
                                l2 = R8;
                                break;
                        case '9':
                                l2 = R9;
                                break;
                        default:
                                break;
                }
        }
	if(l1 == l2){
		return line_number;
	}
	else{
		return 0;
	}

}

long sti(char *str){
	long d = strtod(str, NULL);
//    	printf("d = %ld\n", d);
	return d;
}

int chooseType(char *op){
	if(strcmp(op,"ADD")==0){
		return 1;
	}
	else if(strcmp(op,"SUB")==0){
		return 2;
	}
	else if(strcmp(op,"MUL")==0){
		return 3;
	}
	else if(strcmp(op,"DIV")==0){
		return 4;
	}
	else if(strcmp(op,"MOV")==0){
		return 5;
	}
	else if(strcmp(op,"LW")==0){
                return 6;
        }
	else if(strcmp(op,"SW")==0){
		return 7;
	}
	else if(strcmp(op,"RST")==0){
		return 8;
	}
	else if(strcmp(op,"JMP")==0){
                return 9;
        }
	else if(strcmp(op,"BEQ")==0){
		return 10;
	}
	else if(strcmp(op,"BNE")==0){
		return 11;
	}
	else if(strcmp(op,"SLT")==0){
		return 12;
	}
	return 0;
}


long calc(int type,long x,long y ){
	long result;
	switch (type)
	{
		case 1:
//			printf("add!\n");
			result = add(x,y);
			break;
		case 2:
//			printf("sub!\n");
			result = sub(x,y);
			break;
		case 3:
//			printf("mul!\n");
			result = mul(x,y);
			break;
		case 4:
//			printf("div!\n");
			result = divv(x,y);
			break;
		default:
			break;

	}
	return result;
}




long add(long x, long y){
	return x+y;
}

long sub(long x, long y){
	return x-y;
}

long mul(long x, long y){
	return x*y;
}

long divv(long x, long y){
	if(y == 0){
//		printf("can't divide with 0\n");
		return 0;
	}
	else{
		return x/y;
	}
}

int main(void){

	printf("\nline : %d\n", getLine());
	int totalLine = getLine();
	readFile();
	return 0;

}
