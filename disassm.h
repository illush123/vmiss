#ifndef disassm_h
#define disassm_h

#include <stdio.h>

typedef struct {
    int d;
    int w;
    int mod;
    int reg;
    int r_m;
    int s;
    int v;
    int z;
    unsigned char disp[2];
    unsigned char data[2];
    int opsize;
    unsigned char byte[8];
    char operand[2][16];
    char opcode[16];
    char *asem[4];
    int operand_flag[2];
} Operation;

int get_bit(int from,int to,int byte);
void get_operation(Operation *op);
void print_bynary(Operation *op);
void print_operand(Operation *op, char *opstr);
void determine_mod(Operation *op);
void determine_reg(Operation *op,int flag);
void determine_rm(Operation *op);
unsigned char get_text(void);


#endif /* disassm_h */
