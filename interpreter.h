#ifndef interpreter_h
#define interpreter_h

#include <stdio.h>
#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "disassm.h"

#define MR vm.mutable
#define AX MR.gregs.r16[0]
#define BX MR.gregs.r16[1]
#define CX MR.gregs.r16[2]
#define DX MR.gregs.r16[3]
#define AL MR.gregs.r8[0]
#define AH MR.gregs.r8[1]
#define BL MR.gregs.r8[2]
#define BH MR.gregs.r8[3]
#define CL MR.gregs.r8[4]
#define CH MR.gregs.r8[5]
#define DL MR.gregs.r8[6]
#define DH MR.gregs.r8[7]
#define SP MR.regs[0]
#define BP MR.regs[1]
#define SI MR.regs[2]
#define DI MR.regs[3]
#define IP MR.ip
#define FLAGS MR.flags
#define dmem MR.data_mem
#define IR vm.immutable
#define tsize IR.text_size
#define dsize IR.data_size
#define tmem IR.text_mem
#define CF_BIT 0
#define ZF_BIT 6
#define SF_BIT 7
#define OF_BIT 11

typedef struct VM{
    struct mutable_resource{
        unsigned char data_mem[0x10000];
        union {
            unsigned short r16[4];
            unsigned char r8[8];
        } gregs;
        unsigned short regs[4];
        unsigned short ip, flags;
    } mutable;
    
    struct immutable_resouce{
        int text_size;
        int data_size;
        unsigned char text_mem[0x10000];
    } immutable;
} VM;


typedef struct Storage {
    enum {
        MEM_8BIT,
        REG_8BIT,
        MEM_16BIT,
        REG_16BIT,
        IMM
    } type;
    int index;
} Storage;


extern VM vm;
extern int immediate;
extern char fd_tbl[0xffff];
extern char fdflag;

/*extern char opstr[256];
extern char binary[256];
extern int immediate;
extern int exec_addr;
extern int currentpid;
 
extern char fdflag;
extern char fd_tbl[0xffff];
 */

void change_flag(int answer);

void set_stack(int argc, char *argv[], char *env);

int minix_syscall(void);

void print_resource(void);

int check_reg(Operation *op, int i);

int check_addr(Operation *op);

void exec(Operation *op);

Storage check_operand(Operation *op, int i);

int read_data(Storage st);

void write_data(Storage st, int data);

int read_stack(int offset);

void write_stack(int offset, int data);

void vm_init(void);


#endif

#endif /* interpreter_h */
