#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include <errno.h>
#include <signal.h>
#include "disassm.h"
#include<unistd.h>
#include "message.h"
#include "interpreter.h"
#include "util.h"



VM vm = {0};
int        immediate = 0;
char fd_tbl[0xffff];
char fdflag = 0;
int mapflg = 1;

char apath[256];
const char *rootpath = "/usr/local/core/minix2";

void set_flag(Operation *op, int result);
void set_flag8(int value);
void set_flag16(int value);
char *make_path(char *path);
int read_flags(int index);
void write_flags(int flg, int index);




int read_data(Storage st) {
    int i = st.index;
    switch (st.type) {
        case MEM_8BIT:
            if (0 <= i && i < sizeof(dmem)) return dmem[i];
        case MEM_16BIT:
            if (0 <= i && i + 1 < sizeof(dmem)) return dmem[i] | (dmem[i + 1] << 8);
            else if (i == sizeof(dmem) - 1) return dmem[i] |  (0xff << 8);
            break;
        case REG_8BIT:
            switch (st.index) {
                case 0b000:
                    return AL;
                case 0b001:
                    return CL;
                case 0b010:
                    return DL;
                case 0b011:
                    return BL;
                case 0b100:
                    return AH;
                case 0b101:
                    return CH;
                case 0b110:
                    return DH;
                case 0b111:
                    return BH;
            }
            break;
        case REG_16BIT:
            switch (st.index) {
                case 0b000:
                    return AX;
                case 0b001:
                    return CX;
                case 0b010:
                    return DX;
                case 0b011:
                    return BX;
                case 0b100:
                    return SP;
                case 0b101:
                    return BP;
                case 0b110:
                    return SI;
                case 0b111:
                    return DI;
            }
            break;
        case IMM:
            return i & 0xffff;
    }
    fprintf(stderr, "error:read_data, st.index=0x%04x, st.type=%d", st.index, st.type);
    exit(1);
}

void write_data(Storage st, int data) {
    int i = st.index;
    switch (st.type) {
        case MEM_8BIT:
            dmem[i] = data & 0xff;
            return;
        case MEM_16BIT:
            if (0 <= i && i < sizeof(dmem)) dmem[i] = data & 0xffff;
            if (i + 1 < sizeof(dmem))dmem[i + 1] = (data >> 8) & 0xff;
            return;
        case REG_8BIT:
            switch (st.index) {
                case 0b000:
                    AL = data;
                    break;
                case 0b001:
                    CL = data;
                    break;
                case 0b010:
                    DL = data;
                    break;
                case 0b011:
                    BL = data;
                    break;
                case 0b100:
                    AH = data;
                    break;
                case 0b101:
                    CH = data;
                    break;
                case 0b110:
                    DH = data;
                    break;
                case 0b111:
                    BH = data;
                    break;
                default:
                    fprintf(stderr, "error:write_data (REG8), index = %d", st.index);
                    exit(1);
            }break;
        case REG_16BIT:
            switch (st.index) {
                case 0b000:
                    AX = data;
                    break;
                case 0b001:
                    CX = data;
                    break;
                case 0b010:
                    DX = data;
                    break;
                case 0b011:
                    BX = data;
                    break;
                case 0b100:
                    SP = data;
                    break;
                case 0b101:
                    BP = data;
                    break;
                case 0b110:
                    SI = data;
                    break;
                case 0b111:
                    DI = data;
                    break;
                default:
                    fprintf(stderr, "error:write_data (REG16), index = !!!%d!!!", st.index);
                    exit(1);
            }
            return;
        default:
            fprintf(stderr, "error:write_data TYPE");;
            exit(1);
    }
}



void set_stack(int argc, char *argv[], char *env) {
    unsigned short len = 0;
    int i, j, ai;
    unsigned short env_head, env_len = (unsigned short)strlen(env);
    unsigned short argaddrs[10];
    len = env_len;
    len++;
    fd_tbl[0] = 0;//stdin
    fd_tbl[1] = 1;//stdout
    fd_tbl[2] = 2;//stderr
    for (i = 0; i < argc; ++i) {
        len += strlen(argv[i]);
        len++;
    }
    
    if (len % 2) dmem[--SP] = 0;
    dmem[--SP] = '\0';
    for (i = (env_len - 1); i >= 0; --i) {
        dmem[--SP] = env[i];
    }
    env_head = SP;
    for (ai = 0, i = (argc - 1); i >= 0; --i, ++ai) {
        for (j = strlen(argv[i]); j >= 0; --j) {
            dmem[--SP] = argv[i][j];
        }
        argaddrs[ai] = SP;
    }
    dmem[--SP] = 0;
    dmem[--SP] = 0;
    
    dmem[--SP] = env_head >> 8;
    dmem[--SP] = env_head;
    
    dmem[--SP] = 0;
    dmem[--SP] = 0;
    
    for (i = 0; i < argc; ++i) {
        dmem[--SP] = argaddrs[i] >> 8;
        dmem[--SP] = argaddrs[i];
    }
    
    dmem[--SP] = argc >> 8;
    dmem[--SP] = argc;
    
}

void exec(Operation *op) {
    Storage st1;
    Storage st2;
    int temp;
    if (strcmp(op->opcode, "mov") == 0) {
        st1 = check_operand(op, 1);
        st2 = check_operand(op, 2);
        temp = read_data(st2);
        write_data(st1, temp);
    } else if (strcmp(op->opcode, "push") == 0) {
        st1 = check_operand(op, 1);
        temp = read_data(st1);
        SP -= 2;
        st2.type = MEM_16BIT;
        st2.index = SP;
        write_data(st2, temp);
        //fprintf(stderr, "[ffcb] = %02x,[ffcc] = %02x, [ffcd] = %02x, [ffce] = %02x",dmem[0xffcb],dmem[0xffcc],dmem[0xffcd],dmem[0xffce]);
    } else if (strcmp(op->opcode, "pop") == 0) {
        st1 = check_operand(op, 1);
        st2.type = MEM_16BIT;
        st2.index = SP;
        temp = read_data(st2);
        write_data(st1, temp);
        SP += 2;
    } else if (strcmp(op->opcode, "xchg") == 0) {
        st1 = check_operand(op, 1);
        st2 = check_operand(op, 2);
        temp = read_data(st1);
        write_data(st1, read_data(st2));
        write_data(st2, temp);
    }
    /* else if(strcmp(op->opcode,"in")==0){
     
     }
     else if(strcmp(op->opcode,"out")==0){
     
     }
     else if(strcmp(op->opcode,"xlat")==0){
     
     }
     */  else if (strcmp(op->opcode, "lea") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         write_data(st1, st2.index);
     }
    /*    else if(strcmp(op->opcode,"lds")==0){
     
     }
     else if(strcmp(op->opcode,"les")==0){
     
     }
     else if(strcmp(op->opcode,"lahf")==0){
     
     }
     else if(strcmp(op->opcode,"sahf")==0){
     
     }
     else if(strcmp(op->opcode,"pushf")==0){
     
     }
     else if(strcmp(op->opcode,"popf")==0){
     
     }
     */   else if (strcmp(op->opcode, "add") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         temp = read_data(st1) + read_data(st2);
         write_data(st1, temp);
         set_flag(op, temp);
         
     } else if (strcmp(op->opcode, "adc") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         temp = read_data(st1) + read_data(st2) + read_flags(CF_BIT);
         write_data(st1, temp);
         set_flag(op, temp);
         
     } else if (strcmp(op->opcode, "inc") == 0) {
         int inctemp = read_flags(CF_BIT);
         st1 = check_operand(op, 1);
         temp = read_data(st1) + 1;
         write_data(st1, temp);
         set_flag(op, temp);
         write_flags(inctemp,CF_BIT);
     }
    /* else if(strcmp(op->opcode,"aaa")==0){
     
     
     }
     else if(strcmp(op->opcode,"baa")==0){
     
     }*/
     else if (strcmp(op->opcode, "sbb") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         st2 = check_operand(op, 2);
         temp = temp - (read_data(st2) + read_flags(CF_BIT));
         write_data(st1, temp);
         set_flag(op, temp);
         
     } else if (strcmp(op->opcode, "dec") == 0) {
         int cftemp = read_flags(CF_BIT);
         st1 = check_operand(op, 1);
         temp = read_data(st1) - 1;
         write_data(st1, temp);
         set_flag(op, temp);
         write_flags(cftemp,CF_BIT);
         
     } else if (strcmp(op->opcode, "neg") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         write_data(st1, (temp ^ 0xffff) + 1);
         set_flag(op, (temp ^ 0xffff) + 1);
         if (temp == 0) write_flags(0,CF_BIT);
         else
             write_flags(1, CF_BIT);
     }
    //else if(strcmp(op->opcode,"baa")==0){ }
     else if (strcmp(op->opcode, "sub") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         temp = read_data(st1) - read_data(st2);
         write_data(st1, temp);
         set_flag(op, temp);
     }
    //else if(strcmp(op->opcode,"ssb")==0){ }//else if(strcmp(op->opcode,"dec")==0){ }
     else if (strcmp(op->opcode, "cmp") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         int st1temp = read_data(st1);
         int st2temp = read_data(st2);
         temp = st1temp - st2temp;
         set_flag(op, temp);
         write_flags((unsigned short) read_data(st1) < (unsigned short) read_data(st2), CF_BIT);
     }
    /*
     else if(strcmp(op->opcode,"aas")==0){ }
     else if(strcmp(op->opcode,"das")==0){ }
     */
     else if (strcmp(op->opcode, "mul") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (op->w == 0) {
             char altemp = AX;
             int debug = altemp;
             temp = temp * debug;
             st2.type = REG_16BIT;
             st2.index = 0; //ax
         } else {
             st2.type = REG_16BIT;
             st2.index = 0; //ax
             temp = temp * AX;
             DX = temp >> 16;
         }
         write_data(st2, temp);
         if (temp >> 16 == 0){
             write_flags(0, CF_BIT);
             write_flags(0, OF_BIT);
         }
         else{
             write_flags(1, CF_BIT);
             write_flags(1, OF_BIT);
         }
     }
    /*
     * else if(strcmp(op->opcode,"imul")==0){ }
     * else if(strcmp(op->opcode,"aam")==0){ }
     */
     else if (strcmp(op->opcode, "div") == 0) {
         st1 = check_operand(op, 1);
         st2.type = REG_16BIT;
         st2.index = 0; //ax
         temp = read_data(st1);
         int divtemp = AX;
         int remainer = divtemp % temp;
         int quotient = divtemp / temp;
         if (op->w == 1) {
             Storage st3;
             st3.type = REG_16BIT;
             st3.index = 2;//dx
             write_data(st3, remainer);
             write_data(st2, quotient);
         } else {
             write_data(st2, (quotient << 8) + remainer);
         }
     } else if (strcmp(op->opcode, "idiv") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         st2.type = REG_16BIT;
         st2.index = 0;//ax
         int divtemp = read_data(st2);
         int remainer = divtemp % temp;
         int quotient = divtemp / temp;
         if (op->w == 1) {
             Storage st3;
             st3.type = REG_16BIT;
             st3.index = 2;//dx
             write_data(st3, remainer);
             write_data(st2, quotient);
         } else {
             write_data(st2, (quotient << 8) + remainer);
         }
         
     }
    //else if(strcmp(op->opcode,"aad")==0){ }
     else if (strcmp(op->opcode, "cbw") == 0) {
         st1.type = REG_8BIT;
         st1.index = 0;
         temp = read_data(st1);
         if (temp & 0x80)temp |= 0xff00;
         
         st2.type = REG_16BIT;
         st2.index = 0;
         write_data(st2, temp);
     } else if (strcmp(op->opcode, "cwd") == 0) {
         st1.type = REG_16BIT;
         st1.index = 0; //AX
         st2.type = REG_16BIT;
         st2.index = 2; //DX
         temp = read_data(st1);
         int cwdtemp;
         if (temp & 0x8000)cwdtemp = 0xffff;
         else cwdtemp = 0x0000;
         write_data(st2, cwdtemp);
     } else if (strcmp(op->opcode, "not") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         write_data(st1, ~temp);
     } else if (strcmp(op->opcode, "shl") == 0) {
         st1 = check_operand(op, 1);
         if (!strcmp(op->asem[2], "")) {
             temp = read_data(st1) << 1;
             write_data(st1, temp);
             set_flag(op, temp);
             write_flags(temp >> 16, CF_BIT);
             
         } else {
             st2 = check_operand(op, 2);
             temp = read_data(st1) << read_data(st2);
             write_data(st1, temp);
             set_flag(op, temp);
             //write_flags(temp >> (15 + read_data(st2)), CF_BIT);
             write_flags((temp >> 16) & 1, CF_BIT);
             if (read_data(st2) == 1) {
                 write_flags((read_data(st1) & 0x4000) != (read_data(st1) & 0x8000) >> 1, OF_BIT);
             }
         }
     } else if (strcmp(op->opcode, "shr") == 0) {
         st1 = check_operand(op, 1);
         if (!strcmp(op->asem[2], "")) {
             temp = read_data(st1) >> 1;
             write_data(st1, temp);
             set_flag(op, temp);
             write_flags(temp >> 16, CF_BIT);
         } else {
             st2 = check_operand(op, 2);
             temp = read_data(st1) >> read_data(st2);
             write_data(st1, temp);
             set_flag(op, temp);
             write_flags(temp >> (15 + read_data(st2)), CF_BIT);
         }
     } else if (strcmp(op->opcode, "sar") == 0) {
         st1 = check_operand(op, 1);
         signed short stemp = read_data(st1);
         int firstdata = 0;
         int lastdata = 0;
         if (!strcmp(op->asem[2], "")) {
             firstdata = (stemp >> 15) & 0x1;
             lastdata = stemp & 0x1;
             stemp = stemp >> 1;
             stemp = stemp | firstdata << 15;
             write_data(st1, stemp);
             set_flag(op, stemp);
             write_flags(lastdata, CF_BIT);
         } else {
             
             st2 = check_operand(op, 2);
             int sartemp = read_data(st2);
             firstdata = (stemp >> 15) & 0x1;
             for (int i = 0; i < sartemp; i++) {
                 lastdata = stemp & 0x1;
                 stemp = stemp >> 1;
             }
             if(firstdata){
                 
             }
             stemp = stemp | firstdata << 15;
             write_data(st1, stemp);
             set_flag(op, stemp);
             write_flags(lastdata, CF_BIT);
         }
         
     }
    /*
     * else if(strcmp(op->opcode,"rol")==0){ }
     * else if(strcmp(op->opcode,"ror")==0){ }
     */
     else if (strcmp(op->opcode, "rcl") == 0) {
         if (op->w == 1) {
             st1 = check_operand(op, 1);
             st2 = check_operand(op, 2);
             short rcltemp = read_data(st1);
             temp = rcltemp & 0x07fff;
             temp = temp << 1;
             temp = temp + read_flags(CF_BIT);
             write_data(st1, temp);
             
             write_flags((rcltemp & 0x8000) >> 15 , CF_BIT);
             if (read_data(st2) == 1) {
                 write_flags((read_data(st1) & 0x4000) != (read_data(st1) & 0x8000) >> 1, OF_BIT);
             }
         }
     } else if (strcmp(op->opcode, "rcr") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         short rcltemp = read_data(st1);
         temp = rcltemp & 0xfffe;
         temp = temp >> 1;
         temp = temp + (read_flags(CF_BIT) << 7);
         write_data(st1, temp);
         
         write_flags((rcltemp & 0x8000) >> 15, CF_BIT);
         if (read_data(st2) == 1) {
             write_flags((temp & 0x4000) != (temp & 0x8000) >> 1, OF_BIT);
         }
         
     } else if (strcmp(op->opcode, "and") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         temp = read_data(st1) & read_data(st2);
         write_data(st1, temp);
         set_flag(op, temp);
         write_flags(0, CF_BIT);
         write_flags(0, OF_BIT);
     } else if (strcmp(op->opcode, "test") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         temp = read_data(st1) & read_data(st2);
         set_flag(op, temp);
         write_flags(0, CF_BIT);
         write_flags(0, OF_BIT);
         
     } else if (strcmp(op->opcode, "or") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         temp = read_data(st1) | read_data(st2);
         write_data(st1, temp);
         set_flag(op, temp);
         write_flags(0, CF_BIT);
         write_flags(0, OF_BIT);
     } else if (strcmp(op->opcode, "xor") == 0) {
         st1 = check_operand(op, 1);
         st2 = check_operand(op, 2);
         temp = read_data(st1) ^ read_data(st2);
         write_data(st1, temp);
         set_flag(op, temp);
         write_flags(0, CF_BIT);
         write_flags(0, OF_BIT);
     } else if (strcmp(op->opcode, "rep") == 0) {
         int equal = 0;
         if (!strcmp(op->asem[1], "insb")) {}
         
         else if (!strcmp(op->asem[1], "movsb")) {
             st1.type = MEM_8BIT;
             st2.type = MEM_8BIT;
             while (CX != 0) {
                 st1.index = DI;
                 st2.index = SI;
                 temp = read_data(st2);
                 write_data(st1, temp);
                 SI = (SI + 1) & 0xffff;
                 DI = (DI + 1) & 0xffff;
                 CX--;
             }
         } else if (!strcmp(op->asem[1], "movsw")) {
             st1.type = MEM_16BIT;
             st2.type = MEM_16BIT;
             while (CX != 0) {
                 st1.index = DI;
                 st2.index = SI;
                 temp = read_data(st2);
                 write_data(st1, temp);
                 SI = (SI + 2) & 0xffff;
                 DI = (DI + 2) & 0xffff;
                 CX--;
             }
         } else if (!strcmp(op->asem[1], "scasb")) {
             while (CX != 0 && equal == 0) {
                 st1.type = MEM_8BIT;
                 st1.index = DI;
                 temp = (char) AX - read_data(st1);
                 set_flag(op, temp);
                 DI++;
                 CX--;
                 if (temp == 0) {
                     equal = 1;
                 }
             }
         } else if (!strcmp(op->asem[1], "scasw")) {}
         else if (!strcmp(op->asem[1], "stosb")) {
             while (CX != 0) {
                 st1.type = MEM_8BIT;
                 st1.index = DI;
                 temp = (char) AX;
                 write_data(st1, temp);
                 DI = (DI + 1) & 0xffff;
                 CX--;
             }
         }
         
     } else if (strcmp(op->opcode, "cmpsb") == 0) {
         st1.type = MEM_8BIT;
         st1.index = SI;
         st2.type = MEM_8BIT;
         st2.index = DI;
         temp = read_data(st1) - read_data(st2);
         set_flag(op, temp);
         SI++;
         DI++;
         write_flags((unsigned short) read_data(st1) < (unsigned short) read_data(st2), CF_BIT);
         
     } else if (strcmp(op->opcode, "cmpsw") == 0) {
         st1.type = MEM_16BIT;
         st1.index = SI;
         st2.type = MEM_16BIT;
         st1.index = DI;
         temp = read_data(st1) - read_data(st2);
         set_flag(op, temp);
         SI++;
         DI++;
     }
    /*
     * else if(strcmp(op->opcode,"scas")==0){ }
     * else if(strcmp(op->opcode,"lods")==0){ }
     */
     else if (strcmp(op->opcode, "stosb") == 0) {
         st1.type = MEM_8BIT;
         st1.index = DI;
         temp = (char) AX;
         write_data(st1, temp);
         DI = (DI + 1) & 0xffff;
     } else if (strcmp(op->opcode, "stosw") == 0) {}
     else if (strcmp(op->opcode, "call") == 0) {
         SP -= 2;
         st2.type = MEM_16BIT;
         st2.index = SP;
         write_data(st2, IP);
         st1 = check_operand(op, 1);
         IP = read_data(st1);
         IP &= 0xffff;
     } else if (!strcmp(op->opcode, "jmp short")) {
         st1 = check_operand(op, 2);
         temp = read_data(st1);
         IP = temp;
         IP &= 0xffff;
     } else if (!strcmp(op->opcode, "jmp")) {
         st1 = check_operand(op, 1);
         IP = read_data(st1);
         IP &= 0xffff;
     } else if (strcmp(op->opcode, "ret") == 0) {
         st2.type = MEM_16BIT;
         st2.index = SP;
         temp = read_data(st2);
         SP += 2;
         IP = temp;
         if (strcmp(op->asem[1], "")) {
             st1 = check_operand(op, 1);
             SP += read_data(st1);
         }
     } else if (strcmp(op->opcode, "jnb") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (read_flags(CF_BIT) == 0)IP = temp;
     } else if (strcmp(op->opcode, "je") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (read_flags(ZF_BIT) == 1) IP = temp;
     } else if (strcmp(op->opcode, "jne") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (read_flags(ZF_BIT) == 0)IP = temp;
     } else if (strcmp(op->opcode, "jl") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (read_flags(SF_BIT) != read_flags(OF_BIT))IP = temp;
     } else if (strcmp(op->opcode, "jle") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (read_flags(ZF_BIT) == 1 || read_flags(SF_BIT) != read_flags(OF_BIT))IP = temp;
     } else if (strcmp(op->opcode, "jnl") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (read_flags(SF_BIT) == read_flags(OF_BIT))IP = temp;
     } else if (strcmp(op->opcode, "jnle") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (read_flags(ZF_BIT) == 0 && read_flags(SF_BIT) == read_flags(OF_BIT))IP = temp;
     } else if (strcmp(op->opcode, "jb") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if(read_flags(CF_BIT) == 1) IP = temp;
     } else if (strcmp(op->opcode, "jbe") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (read_flags(CF_BIT)|| read_flags(ZF_BIT)) IP = temp;
         
     } else if (strcmp(op->opcode, "jnbe") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (read_flags(CF_BIT) == 0 && read_flags(ZF_BIT) == 0) IP = temp;
     }
    /*else if(strcmp(op->opcode,"jp/jpe")==0){ }
     else if(strcmp(op->opcode,"jo")==0){ }
     else if(strcmp(op->opcode,"js")==0){ }
     else if(strcmp(op->opcode,"jno")==0){ }
     else if(strcmp(op->opcode,"jns")==0){ }
     */else if (strcmp(op->opcode, "loop") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         CX--;
         if (CX != 0)
             IP = temp;
         
     } else if (strcmp(op->opcode, "jcxz") == 0) {
         if (CX == 0) {
             st1 = check_operand(op, 1);
             IP = read_data(st1);
         }
         
     } else if (strcmp(op->opcode, "int") == 0) {
         st1 = check_operand(op, 1);
         temp = read_data(st1);
         if (temp == 0x20)
             minix_syscall();
         else fprintf(stderr, "undefined\n");
         AX = 0;
     }
    /*
     * else if(strcmp(op->opcode,"clc")==0){ }
     * * else if(strcmp(op->opcode,"cmc")==0){ }
     * * else if(strcmp(op->opcode,"stc")==0){ }
     */
     else if (strcmp(op->opcode, "cld") == 0) {}
    /*
     * else if(strcmp(op->opcode,"std")==0){
     * else if(strcmp(op->opcode,"cli")==0){ }
     * else if(strcmp(op->opcode,"sti")==0){ }
     * else if(strcmp(op->opcode,"hlt")==0){ }
     * else if(strcmp(op->opcode,"wait")==0){ }
     * else if(strcmp(op->opcode,"esc")==0){ }
     * else if(strcmp(op->opcode,"lock")==0){ }
     */
     else {
         fprintf(stderr, "未実装");
         exit(1);
     }
}
void change_flag(int answer) {
    if (answer < 0)write_flags(1, CF_BIT);
    if (answer == 0)write_flags(1, OF_BIT);
    write_flags(get_bit(1, 1, answer), ZF_BIT);
}

int minix_syscall() {
    message *m = (message *) &dmem[BX];
    Storage st;
    int status;
    switch (m->m_type) {
        case 1://exit命令
            status = dmem[BX + 4]; //m.m1_i1
            fprintf(stderr, "\n<exit(%d)>", status);
            /*
             currentpid--;
             if (currentpid - ID < 0) {
             exit(status);
             } else {
             status = currentpid + 1;
             fprintf(stderr, "\n<wait()> => %d, 0x%x", currentpid + 1, currentpid - ID);
             }
             */
            
            //exit(status);
            exit(0);
            break;
            /*
             case 2://fork
             memcpy(&hw[currentpid - ID + 1], &hw[currentpid - ID], sizeof hw[currentpid - ID]);
             fprintf(stderr, "\n<fork() => %d>", currentpid + 1);
             status = currentpid + 1;
             break;
             */
        case 3://read命令
        {
            st.type = MEM_16BIT;
            st.index = BX + 4;
            int fd = read_data(st);
            st.index = BX + 6;
            int nbyte = read_data(st);
            st.index = BX + 10;
            int buf = read_data(st);
            if(mapflg){
              fprintf(stderr, "\n<read(%d, 0x%04x, %d)", fd_tbl[fd], buf, nbyte);
              status = read(fd_tbl[fd], &dmem[buf], nbyte);
            }else{
              fprintf(stderr, "\n<read(%d, 0x%04x, %d)",fd, buf, nbyte);
              status = read(fd, &dmem[buf], nbyte);
            } 
            if(status == -1){
              perror("read");
              exit(1);
            } 
            fprintf(stderr, " => %d>", status);
        }
            break;
        case 4://write命令
            st.type = MEM_16BIT;
            st.index = BX + 4;
            int fd = read_data(st);
            st.index = BX + 6;
            int nbyte = read_data(st);
            st.index = BX + 10;
            int buf = read_data(st);
            fprintf(stderr, "\n<write(%d, 0x%04x, %d)", fd_tbl[fd], buf, nbyte);
            status = write(fd_tbl[fd], &dmem[buf], nbyte);
            if(status == -1){
              perror("write");
              exit(1);
            } 
            fprintf(stderr, " => %d>", status);
            break;
        case 5: {//open
            fdflag = 1;
            st.type = MEM_16BIT;
            st.index = BX + 4;
            unsigned int len = read_data(st);
            st.index = BX + 6;
            unsigned int flag = read_data(st);
            if (flag & O_CREAT) {
                int mode_t = dmem[BX + 8];
                int addr = dmem[BX + 10];
            } else {
                unsigned int addr;
                st.index = BX + 8;
                (len <= 14) ? (addr = BX + 10) : (addr = read_data(st));
                status = open(make_path((char *) &dmem[addr]), flag);
                if(status == -1){
                  perror("open");
                  exit(1);
                }
                fprintf(stderr, "\n<open(\"%s\", %d)", &dmem[addr], flag);
                fprintf(stderr, " => %d>", status);
            }
        }
            break;
        case 6: {
            int fd = m->m1_i1;
            if(mapflg){
              fprintf(stderr, "\n<close(%d)", fd_tbl[fd]);
              status = close(fd_tbl[fd]);
            }else{
              fprintf(stderr, "\n<close(%d)", fd);
              status = close(fd);
            }
            if(status == -1){
              perror("close");
              exit(1);
            }
            fprintf(stderr, " => %d>", status);
            break;
        }
            /*
             case 7: {//wait
             fprintf(stderr, "\n<wait()>");
             status = 0;
             currentpid++;
             fprintf(stderr, "currentpid = %d", currentpid);
             st.type = MEM_16BIT;
             st.index = BX + 4;
             write_data(st, status);
             }
             break;
             */
        case 8://creat
            st.type = MEM_16BIT;
            st.index = BX + 4;
            short len = read_data(st);
            st.index = BX + 6;
            unsigned short mode = read_data(st);
            unsigned short addr;
            if (len <= 14) {
                addr = BX + 10;
            } else {
                st.index = BX + 8;
                addr = read_data(st);
            }
            fprintf(stderr, "\n<creat(\"%s\", 0%03o) ", &dmem[addr], mode);
            status = creat(make_path((char *) &dmem[addr]), mode);
            fprintf(stderr, "=> %d>", status);
            break;
        case 10: {//unlink
            int len;
            int addr;
            st.type = MEM_16BIT;
            st.index = BX + 4;
            len = read_data(st);
            
            if (len <= 14) {
                addr = BX + 10;
            } else {
                st.index = BX + 8;
                addr = read_data(st);
            }
            fprintf(stderr, "\n<unlink(\"%s\")", &dmem[addr]);
            status = unlink(make_path((char *) &dmem[addr]));
            fprintf(stderr, " => %d>", status);
        }
            break;
        case 13://time
            fprintf(stderr, "\n<time()");
            int ret = 0x54705002;
            st.type = MEM_16BIT;
            //
            st.index = BX + 10;
            write_data(st, ret);
            st.index = BX + 12;
            write_data(st, ret >> 16);
            //
            fprintf(stderr, " => %d>", ret);
            status = 0;
            break;
        case 15: { //chmod
            int len;
            int mode;
            int addr;
            
            st.type = MEM_16BIT;
            st.index = BX + 4;
            len = read_data(st);
            st.index = BX + 6;
            mode = read_data(st);
            if (len <= 14) {
                addr = BX + 10;
            } else {
                st.index = BX + 8;
                addr = read_data(st);
            }
            fprintf(stderr, "\n<chmod(\"%s\", 0%03o)", &dmem[addr], mode);
            status = chmod(make_path((char *) &dmem[addr]), mode);
            fprintf(stderr, " => %d>", status);
        }
            break;
        case 17://brk
            
        {
            st.type = MEM_16BIT;
            st.index = BX + 10;
            int addr = read_data(st);
            fprintf(stderr, "\n<brk(0x%04x) => 0>", addr);
            status = 0;
            m->m_type = status;
            st.type = MEM_16BIT;
            st.index = BX + 18;
            write_data(st, addr);
        }
            break;
        case 19:
        {
            st.type = MEM_16BIT;
            st.index = BX + 4;
            int fd = read_data(st);
            st.index = BX + 10;
            int offset = read_data(st);
            st.index = BX + 6;
            int whence = read_data(st);
            status = lseek(fd_tbl[fd], offset, whence);
            if(status == -1){
              perror("lseek");
              exit(1);
            }
            fprintf(stderr, "\n<lseek(%d, %d, %d)", fd_tbl[fd], offset, whence);
            fprintf(stderr, " => %d>", status);
            st.type = MEM_16BIT;
            st.index = (BX + 10);
            write_data(st, status);
            status = 0;
        }
            break;
            /*
             case 20://getpid()
             fprintf(stderr, "\n<getpid()");
             status = currentpid;
             fprintf(stderr, " => %d>", currentpid);
             break;
             */
        case 33://access;
        {
            st.type = MEM_16BIT;
            
            st.index = BX + 4;
            unsigned short len = read_data(st);
            
            st.index = BX + 6;
            unsigned short mode = read_data(st);
            unsigned short addr;
            if (len <= 14)
                addr = BX + 10;
            else {
                st.index = BX + 8;
                read_data(st);
            }
            fprintf(stderr, "\n<access(\"%s\", 0%03o)", &dmem[addr], mode);
            char *path1;
            path1 = (char *) &dmem[addr];
            int result = access(make_path(path1), mode);
            fprintf(stderr, " => %d>", result);
            if (result == -1)
                status = -errno;
        }
            break;
            
            
        case 54://ioctl
        {
            st.type = MEM_16BIT;
            
            st.index = BX + 4;
            int fd = read_data(st);
            
            st.index = BX + 8;
            int request = read_data(st);
            
            st.index = BX + 18;
            int address = read_data(st);
            
            status = -EINVAL & 0xffff;//errno 22 invalid argument
            fprintf(stderr, "\n<ioctl(%d, 0x%04x, 0x%04x)>", fd, request, address);
        }
            break;
        case 59://exec
            break;
            /*
             st.type = MEM_16BIT;
             st.index = BX + 10;
             int path = read_data(st);
             
             st.index = BX + 12;
             int fddr = read_data(st);
             
             int index = fddr + 2;
             char *execpath;
             execpath = make_path((char *) &dmem[path]);
             
             //あとでmalloc
             int count = 0;
             int argc = 0;
             char **argv;
             argv = (char **) malloc(1000);
             while (1) {
             st.index = index;
             int argaddr = read_data(st);
             if (argaddr == 0)break;
             argv[count] = (char *) malloc(256);
             strcpy(argv[count], (char *) &dmem[(argaddr + fddr)]);
             index += 2;
             count++;
             argc++;
             }
             index += 2;
             char **envp;
             envp = (char **) malloc(100);
             count = 0;
             while (1) {
             st.index = index;
             int envaddr = read_data(st);
             if (envaddr == 0)break;
             envp[count] = (char *) malloc(100);
             strcat(envp[count], (char *) &dmem[envaddr + fddr]);
             index += 2;
             count++;
             }
             fprintf(stderr, "\n<exec(\"%s\"", (char *) &dmem[path]);
             for (int i = 1; i < argc; ++i) {
             fprintf(stderr, ", \"%s\"", argv[i]);
             }
             fprintf(stderr, ")>");
             memset(tmem, 0, 0x10000);
             memset(dmem, 0, 0x10000);
             char str1[256];
             strcpy(str1, make_path(argv[0]));
             fprintf(stderr, "path=%s", str1);
             unsigned char *execmem = create_data_from_file(str1);
             tsize = execmem[8] + (execmem[9] << 8) + (execmem[10] << 16) + (execmem[11] << 24);
             int execdsize = execmem[12] + (execmem[13] << 8) + (execmem[14] << 16) + (execmem[15] << 24);
             memcpy(tmem, &execmem[0x20], tsize);
             memcpy(dmem, &execmem[0x20 + tsize], execdsize);
             destroy_data(execmem);
             IP = 0;
             SP = 0;
             set_stackexec(argc, argv, envp[0]);
             break;
             */
        case 71://sigaction
            break;
            /*
             st.type = MEM_16BIT;
             st.index = BX + 6;
             int sig = read_data(st);
             
             st.index = BX + 10;
             int act = read_data(st);
             
             st.index = BX + 12;
             int oact = read_data(st);
             fprintf(stderr, "\n<sigaction(%d, 0x%04x, 0x%04x)>", sig, act, oact);
             int s = 0;
             switch (sig) {
             case 2:
             s = SIGINT;
             status = 0;
             break;
             case 4:
             s = SIGILL;
             status = 0;
             break;
             case 8:
             s = SIGFPE;
             status = 0;
             break;
             case 11:
             s = SIGSEGV;
             status = 0;
             break;
             default:
             status = -EINVAL & 0xffff;
             break;
             }
             if (oact) {
             st.index = oact;
             write_data(st, sigary[sig].handler);
             st.index = oact + 2;
             write_data(st, (unsigned short) sigary[sig].mask);
             st.index = oact + 6;
             }
             st.index = act;
             sigary[sig].handler = read_data(st);
             st.index = act + 2;
             sigary[sig].mask = read_data(st);//本当は32bit
             st.index = act + 6;
             sigary[sig].flag = read_data(st);
             break;
             */
        default:
            fprintf(stderr, "未実装のシステムコールですbx=%d,case=%d\n", BX, m->m_type);
            break;
    }
    m->m_type = status;
    if (m->m_type < 0) {
        fprintf(stderr, "exit(-1)");
        exit(-1);
        return -1;
    }
    return m->m_type;
}

void print_resource() {
    char flag[5] = "----";
    if (read_flags(OF_BIT))flag[0] = 'O';
    if (read_flags(SF_BIT))flag[1] = 'S';
    if (read_flags(ZF_BIT))flag[2] = 'Z';
    if (read_flags(CF_BIT))flag[3] = 'C';
    fprintf(stderr, "%04x %04x %04x %04x %04x %04x %04x %04x %s ", AX, BX, CX, DX, SP, BP, SI, DI, flag);
}


int check_reg(Operation *op, int i) {
    //REG16
    if (strcmp(op->asem[i], "ax") == 0) return 0b000;
    else if (strcmp(op->asem[i], "cx") == 0)return 0b001;
    else if (strcmp(op->asem[i], "dx") == 0)return 0b010;
    else if (strcmp(op->asem[i], "bx") == 0)return 0b011;
    else if (strcmp(op->asem[i], "sp") == 0)return 0b100;
    else if (strcmp(op->asem[i], "bp") == 0)return 0b101;
    else if (strcmp(op->asem[i], "si") == 0)return 0b110;
    else if (strcmp(op->asem[i], "di") == 0)return 0b111;
    //REG8
    else if (strcmp(op->asem[i], "al") == 0)return 0b000;
    else if (strcmp(op->asem[i], "cl") == 0)return 0b001;
    else if (strcmp(op->asem[i], "dl") == 0)return 0b010;
    else if (strcmp(op->asem[i], "bl") == 0)return 0b011;
    else if (strcmp(op->asem[i], "ah") == 0)return 0b100;
    else if (strcmp(op->asem[i], "ch") == 0)return 0b101;
    else if (strcmp(op->asem[i], "dh") == 0)return 0b110;
    else if (strcmp(op->asem[i], "bh") == 0)return 0b111;
    //後で改行部分修正する
    else fprintf(stderr, "error:asem[%d][2]=%x\n", i, op->asem[i][2]);
    return -1;
}

int check_addr(Operation *op) {
    unsigned short ea;
    switch (op->r_m) {
        case 0:
            ea = BX + SI;
            break;
        case 1:
            ea = BX + DI;
            break;
        case 2:
            ea = BP + SI;
            break;
        case 3:
            ea = BP + DI;
            break;
        case 4:
            ea = SI;
            break;
        case 5:
            ea = DI;
            break;
        case 6:
            ea = BP;
            break;
        case 7:
            ea = BX;
            break;
        default :
            if(op->w==1)
                fprintf(stderr," ;[%04x]%02x%02x",(op->disp[1]<<8)+op->disp[0],dmem[(op->disp[1]<<8)+op->disp[0]+1],dmem[(op->disp[1]<<8)+op->disp[0]]);
            else
                fprintf(stderr," ;[%04x]%02x",(op->disp[1]<<8)+op->disp[0],dmem[(op->disp[1]<<8)+op->disp[0]]);
            return (op->disp[1] << 8) + op->disp[0];
    }
    int addr = 0;
    switch (op->mod) {
        case 0:
            if (op->r_m == 6) {
                addr = ea + (op->disp[1] << 8) + op->disp[0];
            } else addr = ea;
            break;
        case 1:
            if (op->disp[0] >= 0x80)
                addr = ea - (op->disp[0] ^ 0xff) - 1;
            else addr = ea + op->disp[0];
            break;
        case 2:
            addr = ea + (op->disp[1] << 8) + op->disp[0];
            break;
        default:
            fprintf(stderr, "error-checkaddr\n");
            break;
    }
    addr &= 0xffff;
    if (op->w == 1) {
        fprintf(stderr," ;[%04x]%02x%02x",addr,dmem[addr+1],dmem[addr]);
    } else {
        fprintf(stderr," ;[%04x]%02x",addr,dmem[addr]);
    }
    return addr;
}

Storage check_operand(Operation *op, int index) {
    Storage st;
    switch (op->asem[index][0]) {
        case '[':
            if (op->w == 1) {
                st.type = MEM_16BIT;
            } else {
                st.type = MEM_8BIT;
            }
            st.index = check_addr(op);
            break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 's':
            switch (op->asem[index][1]) {
                case 'x':
                case 'l':
                case 'p':
                case 'i':
                case 'h':
                    if (index == 2)op->asem[2][3] = 0;
                    if (op->w == 1) {
                        st.type = REG_16BIT;
                    } else {
                        st.type = REG_8BIT;
                    }
                    st.index = check_reg(op, index);
                    break;
                default:
                    if (op->w == 1) {
                        immediate = op->data[0] + (op->data[1] << 8);
                    } else {
                        immediate = op->data[0];
                        if (immediate >= 0x80)
                            immediate = -(0x100 - op->data[0]);
                    }
                    st.type = IMM;
                    st.index = immediate;
                    break;
            }
            break;
        default:
            immediate = op->data[0] + (op->data[1] << 8);
            st.type = IMM;
            st.index = immediate;
            break;
    }
    return st;
}


int read_flags(int index){
    return (FLAGS >> index) & 1;
}

void write_flags(int flg, int index){
    if(flg){
        FLAGS |= (1 << index);
    }else{
        unsigned short mask = 0xffff ^ (1 << index);
        FLAGS &= mask;
    }
}



void set_flag(Operation *op, int value) {
    if (op->w == 1) {
        set_flag16(value);
    } else {
        set_flag8(value);
    }
}

void set_flag8(int result) {
    char v = result;
    write_flags(v < 0, SF_BIT);
    write_flags((v == 0), ZF_BIT);
    write_flags((result >> 8) & 0x01, CF_BIT);
}

void set_flag16(int result) {
    short v = result;
    write_flags(v < 0,SF_BIT);
    write_flags(v == 0, ZF_BIT);
    write_flags((result >> 16) & 0x01, CF_BIT);
}

int read_stack(int offset) {
    Storage pos;
    pos.type = MEM_16BIT;
    pos.index = (((SP + offset) & 0xffff)) & 0xfffff;
    return read_data(pos);
}

char *make_path(char *path) {
    if (path[0] != '/')
        return path;
    char str[256] = "/usr/tmp";
    char *ret;
    ret = strstr(path, str);
    if (ret != NULL) {
        path[0] = '/';
        path[1] = 't';
        path[2] = 'm';
        path[3] = 'p';
        return path;
    }
    char str2[256] = "/tmp";
    ret = strstr(path, str2);
    if (ret != NULL) {
        path[0] = '/';
        path[1] = 't';
        path[2] = 'm';
        path[3] = 'p';
        return path;
        
    }
    sprintf(apath, "%s%s", rootpath, path);
    return apath;
}

void write_stack(int offset, int data) {
    Storage pos;
    pos.type = MEM_16BIT;
    pos.index = (((SP + offset) & 0xffff)) & 0xfffff;
    write_data(pos, data);
}
