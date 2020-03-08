#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include "disassm.h"
#include "interpreter.h"
#include "x86emu.h"




int get_bit(int from, int to, int byte) {
    if (from > 1 || to < 1)byte = byte & 0x7f;//011111111
    if (from > 2 || to < 2)byte = byte & 0xbf;//101111111
    if (from > 3 || to < 3)byte = byte & 0xdf;//101111111
    if (from > 4 || to < 4)byte = byte & 0xef;
    if (from > 5 || to < 5)byte = byte & 0xf7;
    if (from > 6 || to < 6)byte = byte & 0xfb;
    if (from > 7 || to < 7)byte = byte & 0xfd;
    if (from > 8 || to < 8)byte = byte & 0xfe;
    byte >>= 8 - to;
    return byte;
}


void determine_rm(Operation *op) {
    switch (op->r_m) {
        case 0:
            strcpy(op->operand[1], "[bx+si");
            break;
        case 1:
            strcpy(op->operand[1], "[bx+di");
            break;
        case 2:
            strcpy(op->operand[1], "[bp+si");
            break;
        case 3:
            strcpy(op->operand[1], "[bp+di");
            break;
        case 4:
            strcpy(op->operand[1], "[si");
            break;
        case 5:
            strcpy(op->operand[1], "[di");
            break;
        case 6:
            strcpy(op->operand[1], "[bp");
            break;
        case 7:
            strcpy(op->operand[1], "[bx");
            break;
        default:
            sprintf(op->operand[1], "[%02x%02x]", op->disp[1], op->disp[0]);
            return;
            break;
    }
    char str[16];
    if (op->disp[1] == 0 && op->disp[0] == 0) {
        strcat(op->operand[1], "]");
    } else {
        unsigned char disp0 = 0, disp1 = 0;
        if (op->disp[1] == 0) {
            if (op->disp[0] > 0x80)
                sprintf(str, "-%x]", (op->disp[0] ^ 0xff) + 1);
            else
                sprintf(str, "+%x]", op->disp[0]);
        } else {
            if (op->disp[1] > 0x80) {
                strcat(op->operand[1], "-");
                short temp = (op->disp[1] << 8) + op->disp[0];
                temp = 0x10000 - temp;
                disp1 = (temp >> 8) & 0xff;
                disp0 = temp;
            } else {
                strcat(op->operand[1], "+");
                disp1 = op->disp[1];
                disp0 = op->disp[0];
            }
            if (op->disp[1] == 0) {//表示を合わせるための分岐
                sprintf(str, "%x]", disp0);
            } else {
                sprintf(str, "%x%02x]", disp1, disp0);
            }
        }
        strcat(op->operand[1], str);
    }
}

void determine_reg(Operation *op, int flag) {
    int temp;
    if (flag == 0) {
        temp = op->reg;
    } else {
        temp = op->r_m;
    }
    if (op->w == 1) {
        switch (temp) {
            case 0:
                strcpy(op->operand[flag], "ax");
                break;
            case 1:
                strcpy(op->operand[flag], "cx");
                break;
            case 2:
                strcpy(op->operand[flag], "dx");
                break;
            case 3:
                strcpy(op->operand[flag], "bx");
                break;
            case 4:
                strcpy(op->operand[flag], "sp");
                break;
            case 5:
                strcpy(op->operand[flag], "bp");
                break;
            case 6:
                strcpy(op->operand[flag], "si");
                break;
            case 7:
                strcpy(op->operand[flag], "di");
                break;
        }
    } else if (op->w == 0) {
        switch (temp) {
            case 0:
                strcpy(op->operand[flag], "al");
                break;
            case 1:
                strcpy(op->operand[flag], "cl");
                break;
            case 2:
                strcpy(op->operand[flag], "dl");
                break;
            case 3:
                strcpy(op->operand[flag], "bl");
                break;
            case 4:
                strcpy(op->operand[flag], "ah");
                break;
            case 5:
                strcpy(op->operand[flag], "ch");
                break;
            case 6:
                strcpy(op->operand[flag], "dh");
                break;
            case 7:
                strcpy(op->operand[flag], "bh");
                break;
        }
    } else {
        switch (temp) {
            case 0:
                strcpy(op->operand[flag], "es");
                break;
            case 1:
                strcpy(op->operand[flag], "cs");
                break;
            case 2:
                strcpy(op->operand[flag], "ss");
                break;
            case 3:
                strcpy(op->operand[flag], "ds");
                break;
        }
    }
}

void determine_mod(Operation *op) {
    switch (op->mod) {
        case 0:
            if (op->r_m == 6) {
                op->byte[2] = op->disp[0] = get_text();
                op->opsize++;
                op->byte[3] = op->disp[1] = get_text();
                op->opsize++;
                op->r_m = -1;
                determine_rm(op);
            } else {
                op->disp[0] = 0;
                op->disp[1] = 0;
                determine_rm(op);
            }
            break;
        case 1:
            op->byte[2] = op->disp[0] = get_text();
            op->disp[1] = 0;
            op->opsize++;
            determine_rm(op);
            break;
        case 2:
            op->byte[2] = op->disp[0] = get_text();
            op->opsize++;
            op->byte[3] = op->disp[1] = get_text();
            op->opsize++;
            determine_rm(op);
            break;
        case 3:
            determine_reg(op, 1);
            break;
    }
}

void print_operand(Operation *op,char *opstr) {
    char str1[16];
    char str2[16];
    
    if (op->d == 0) {
        strcpy(str1, op->operand[1]);
        strcpy(str2, op->operand[0]);
    } else if (op->d == 1) {
        strcpy(str1, op->operand[0]);
        strcpy(str2, op->operand[1]);
    } else {
    }
    sprintf(opstr, "%s %s, %s", op->opcode, str1, str2);
}

void print_bynary(Operation *op) {
    for (int i = 0; i < op->opsize; i++) {
        fprintf(stderr, "%02x", op->byte[i]);
    }
    for (int i = 0; i < 6 - op->opsize; i++) {
        fprintf(stderr, "  ");
    }
    fprintf(stderr, " ");
}

void get_operation(Operation *op) {
    for (int i = 1; i < op->opsize; i++) {
        op->byte[i] = get_text();
    }
}

unsigned char get_text() {
    unsigned char byte;
    byte = tmem[IP];
    IP++;
    return byte;
}


