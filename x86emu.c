#include "x86emu.h"
#include "util.h"
#include "disassm.h"
#include "interpreter.h"
#include "vmiss.h"


int run(int argc, char *argv[],char *env){
    unsigned char* mem = create_data_from_file(argv[0]);
    tsize = mem[8] + (mem[9] << 8) + (mem[10] << 16) + (mem[11] << 24);
    dsize = mem[12] + (mem[13] << 8) + (mem[14] << 16) + (mem[15] << 24);
    char opstr[256];
    memcpy(tmem, &mem[0x20], tsize);
    memcpy(dmem, &mem[0x20 + tsize], dsize);
    destroy_data(mem);
    set_stack(argc, argv, env);
    fprintf(stderr, " AX   BX   CX   DX   SP   BP   SI   DI  FLAGS IP\n");
    char opcode[16];
    int clnt_sock = wait_client();
    while(IP <= tsize){
        compare(clnt_sock, &vm, opcode);
        Operation    op = {};
        op.r_m = op.reg = op.w = op.d = -1;
        strcpy(op.opcode,"error");
        print_resource();
        fprintf(stderr, "%04x:", IP);
        op.byte[0] = get_text();
        switch (op.byte[0]) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
                op.opsize = 2;
                strcpy(op.opcode, "add");
                get_operation(&op);
                op.d = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                determine_reg(&op, 0);
                determine_mod(&op);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
            case 0x04:
                //add imm to acc
            case 0x05:
                op.opsize = 2;
                op.reg = 0;
                strcpy(op.opcode, "add");
                op.w = get_bit(8, 8, op.byte[0]);
                determine_reg(&op, 0);
                if (op.w == 1)
                    op.opsize = 3;
                get_operation(&op);
                op.data[0] = op.byte[1];
                op.data[1] = op.byte[2];
                print_bynary(&op);
                sprintf(opstr, "%s %s, %02x%02x", op.opcode, op.operand[0], op.data[1], op.data[0]);
                break;
            case 0x06:
            case 0x07:
                break;
            case 0x08:
                //or reg / mem and reg to either1
            case 0x09:
            case 0x0a:
            case 0x0b:
                op.opsize = 2;
                strcpy(op.opcode, "or");
                get_operation(&op);
                op.d = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                determine_reg(&op, 0);
                determine_mod(&op);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
            case 0x0c:
            case 0x0d:
            case 0x0e:
            case 0x0f:
                break;
            case 0x10:
                //adc Reg./ Memory with Register to Either
            case 0x11:
            case 0x12:
            case 0x13:
                op.opsize = 2;
                strcpy(op.opcode, "adc");
                get_operation(&op);
                op.d = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                determine_reg(&op, 0);
                determine_mod(&op);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
                break;
            case 0x18:
                //ssb reg / memory and register to either
            case        0x19:
            case        0x1a:
            case        0x1b:
                op.        opsize = 2;
                strcpy(op.opcode, "sbb");
                //ssb ? ?231231mmvm ? ??表示を合わせるためsbb ? ??してる
                get_operation(&op);
                op.d = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                determine_reg(&op, 0);
                determine_mod(&op);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
            case 0x1c:
            case 0x1d:
            case 0x1e:
            case 0x1f:
                break;
            case 0x20:
            case 0x21:
            case 0x22:
            case 0x23:
                op.opsize = 2;
                strcpy(op.opcode, "and");
                get_operation(&op);
                op.d = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                determine_reg(&op, 0);
                determine_mod(&op);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
            case 0x24:
            case 0x25:
            case 0x26:
            case 0x27:
                break;
            case 0x28:
                //sub reg / mem and reg tp either
            case 0x29:
            case 0x2a:
            case 0x2b:
                op.opsize = 2;
                strcpy(op.opcode, "sub");
                get_operation(&op);
                op.d = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                determine_reg(&op, 0);
                determine_mod(&op);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
            case 0x2c:
                //sub immediate from accumulator
            case 0x2d:
                strcpy(op.opcode, "sub");
                op.w = get_bit(8, 8, op.byte[0]);
                op.reg = 0;
                //指定なし
                if (op.w == 1) {
                    
                    op.opsize = 3;
                    get_operation(&op);
                    determine_reg(&op, 0);
                    print_bynary(&op);
                    sprintf(opstr, "%s %s, %02x%02x", op.opcode, op.operand[0], op.byte[2], op.byte[1]);
                    op.data[0] = op.byte[1];
                    op.data[1] = op.byte[2];
                } else {
                    op.opsize = 2;
                    get_operation(&op);
                    determine_reg(&op, 0);
                    print_bynary(&op);
                    sprintf(opstr, "%s byte %s, %04x", op.opcode, op.operand[0], op.byte[1]);
                    op.data[0] = op.byte[1];
                }
            case 0x2e:
            case 0x2f:
                break;
            case 0x30:
                //xor-- - reg./ momory and register to either
            case        0x31:
            case        0x32:
            case        0x33:
            case        0x34:
                op.        opsize = 2;
                get_operation(&op);
                op.d = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                strcpy(op.opcode, "xor");
                determine_reg(&op, 0);
                determine_mod(&op);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
            case 0x35:
            case 0x36:
            case 0x37:
                break;
            case 0x38:
                //cmp r / r and m
            case 0x39:
            case 0x3a:
            case 0x3b:
                op.opsize = 2;
                get_operation(&op);
                op.d = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                strcpy(op.opcode, "cmp");
                determine_reg(&op, 0);
                determine_mod(&op);
                print_bynary(&op);
                print_operand(&op,opstr);
                
                break;
            case 0x3c:
                //cmp imm acm
            case 0x3d:
            case 0x3e:
                op.opsize = 1;
                strcpy(op.opcode, "cmp");
                op.w = get_bit(8, 8, op.byte[0]);
                op.reg = 0;
                //ax / al ? ??なる？後で調べる
                if (op.w == 1) {
                    op.opsize = 3;
                    get_operation(&op);
                    determine_reg(&op, 0);
                    print_bynary(&op);
                    sprintf(opstr, "%s %s, %02x%02x", op.opcode, op.operand[0], op.byte[2], op.byte[1]);
                    op.data[0] = op.byte[1];
                    op.data[1] = op.byte[2];
                    //インタプリタよう
                } else {
                    op.opsize = 2;
                    get_operation(&op);
                    determine_reg(&op, 0);
                    print_bynary(&op);
                    sprintf(opstr, "%s byte %s, %04x", op.opcode, op.operand[0], op.byte[1]);
                    op.data[0] = op.byte[1];
                    //インタプリタ用
                }
                break;
            case 0x3f:
            case 0x40:
                //inc register
            case        0x41:
            case        0x42:
            case        0x43:
            case        0x44:   //dec register
            case        0x45:
            case        0x46:
            case        0x47:
                op.        opsize = 1;
                strcpy(op.opcode, "inc");
                op.reg = get_bit(6, 8, op.byte[0]);
                op.w = 1;
                //後でみる
                determine_reg(&op, 0);
                print_bynary(&op);
                sprintf(opstr, "%s %s", op.opcode, op.operand[0]);
                break;
            case 0x48:
            case 0x49:
            case 0x4a:
            case 0x4b:
            case 0x4c:
            case 0x4d:
            case 0x4e:
            case 0x4f:
                op.opsize = 1;
                strcpy(op.opcode, "dec");
                op.reg = get_bit(6, 8, op.byte[0]);
                op.w = 1;
                //後でみる
                determine_reg(&op, 0);
                print_bynary(&op);
                sprintf(opstr, "%s %s", op.opcode, op.operand[0]);
                break;
            case 0x50:
                //push register
            case        0x51:
            case        0x52:
            case        0x53:
            case        0x54:
            case        0x55:
            case        0x56:
            case        0x57:
                op.        opsize = 1;
                op.w = 1;
                //push ? ??時は16 bit ? ??ジスタ
                strcpy(op.opcode, "push");
                op.reg = get_bit(6, 8, op.byte[0]);
                determine_reg(&op, 0);
                print_bynary(&op);
                sprintf(opstr, "%s %s", op.opcode, op.operand[0]);
                break;
            case 0x58:
            case 0x59:
            case 0x5a:
            case 0x5b:
            case 0x5c:
            case 0x5d:
            case 0x5e:
            case 0x5f:
                op.opsize = 1;
                op.w = 1;
                //push ? ??時は16 bit ? ??ジスタ？
                strcpy(op.opcode, "pop");
                op.reg = get_bit(6, 8, op.byte[0]);
                determine_reg(&op, 0);
                print_bynary(&op);
                sprintf(opstr, "%s %s", op.opcode, op.operand[0]);
                break;
            case 0x60:
            case 0x61:
            case 0x62:
            case 0x63:
            case 0x64:
            case 0x65:
            case 0x66:
            case 0x67:
            case 0x68:
            case 0x69:
            case 0x6a:
            case 0x6b:
            case 0x6c:
            case 0x6d:
            case 0x6e:
            case 0x6f:
                break;
            case 0x70:
                //jo
            case 0x71:
                //jno
            case 0x72:
                //jb
            case 0x73:    ;
                //jnb
            case 0x74:
                //je
            case 0x75:
                //jne
            case 0x76:
                //jbe
            case 0x77:
                //jnbe
            case 0x78:
                //js
            case 0x79:
                //jns
            case 0x7a:
                //jp
            case 0x7b:
                //jnp
            case 0x7c:
                //jl
            case 0x7d:
                //jnl
            case 0x7e:
                //jle
            case 0x7f:
                //jnle
                switch (op.byte[0] - 0x70) {
                    case 0:
                        strcpy(op.opcode, "jo");
                        break;
                    case 1:
                        strcpy(op.opcode, "jno");
                        break;
                    case 2:
                        strcpy(op.opcode, "jb");
                        break;
                    case 3:
                        strcpy(op.opcode, "jnb");
                        break;
                    case 4:
                        strcpy(op.opcode, "je");
                        break;
                    case 5:
                        strcpy(op.opcode, "jne");
                        break;
                    case 6:
                        strcpy(op.opcode, "jbe");
                        break;
                    case 7:
                        strcpy(op.opcode, "jnbe");
                        break;
                    case 8:
                        strcpy(op.opcode, "js");
                        break;
                    case 9:
                        strcpy(op.opcode, "jns");
                        break;
                    case 0xa:
                        strcpy(op.opcode, "jp");
                        break;
                    case 0xb:
                        strcpy(op.opcode, "jnp");
                        break;
                    case 0xc:
                        strcpy(op.opcode, "jl");
                        break;
                    case 0xd:
                        strcpy(op.opcode, "jnl");
                        break;
                    case 0xe:
                        strcpy(op.opcode, "jle");
                        break;
                    case 0xf:
                        strcpy(op.opcode, "jnle");
                        break;
                }
                op.opsize = 2;
                get_operation(&op);
                print_bynary(&op);
                /*
                 * if(op.byte[1]>=0x80){
                 * op.byte[1]=-(0x100-op.byte[1]); }
                 */
                op.data[0] = op.byte[1];
                //インタプリタ用
                int        jmp_temp;
                
                if (0xa0 <= op.byte[1])
                    jmp_temp = IP - 0x100 + op.byte[1];
                else
                    jmp_temp = op.data[0] + IP;
                sprintf(opstr, "%s %04x", op.opcode, jmp_temp);
                op.data[0] = jmp_temp;
                op.data[1] = jmp_temp >> 8;
                break;
            case 0x80:
                //immediate with refister / memory CMP
            case 0x81:
            case 0x82:
            case 0x83:
                op.s = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.opsize = 2;
                get_operation(&op);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                determine_mod(&op);
                op.opsize++;
                switch (get_bit(3, 5, op.byte[1])) {
                    case 0:
                        strcpy(op.opcode, "add");
                        break;
                    case 1:
                        strcpy(op.opcode, "or");
                        break;
                    case 3:
                        strcpy(op.opcode, "sbb");
                        break;
                    case 4:
                        strcpy(op.opcode, "and");
                        break;
                    case 5:
                        strcpy(op.opcode, "sub");
                        break;
                    case 7:
                        strcpy(op.opcode, "cmp");
                        break;
                    default:
                        sprintf(opstr, "defaultです\n");
                        break;
                }
                op.byte[op.opsize - 1] = op.data[0] = get_text();
                if (op.s == 0 && op.w == 1) {
                    //１６ビットのデータ読み込??287
                    op.opsize++;
                    op.byte[op.opsize - 1] = op.data[1] = get_text();
                    print_bynary(&op);
                    sprintf(opstr, "%s %s, %02x%02x", op.opcode, op.operand[1], op.data[1], op.data[0]);
                } else if (op.s == 1 && op.w == 1) {
                    print_bynary(&op);
                    op.data[1] = 0;
                    if (op.data[0] >= 0x80) {
                        sprintf(opstr, "%s %s, -%x", op.opcode, op.operand[1], 0x100 - op.data[0]);
                        op.data[1] = 0xff;
                    } else
                        sprintf(opstr, "%s %s, %x", op.opcode, op.operand[1], op.data[0]);
                    
                } else {
                    print_bynary(&op);
                    sprintf(opstr, "%s byte %s, %x", op.opcode, op.operand[1], op.data[0]);
                }
                break;
            case 0x84:
                //test Register / Memory and Register
            case 0x85:
                op.opsize = 2;
                get_operation(&op);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                strcpy(op.opcode, "test");
                determine_mod(&op);
                determine_reg(&op, 0);
                print_bynary(&op);
                sprintf(opstr, "%s %s, %s", op.opcode, op.operand[0], op.operand[1]);
                break;
            case 0x86:
                //xchg Register / Memory with Register
            case 0x87:
                op.opsize = 2;
                get_operation(&op);
                op.d = 0;
                //from reg
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                strcpy(op.opcode, "xchg");
                determine_mod(&op);
                determine_reg(&op, 0);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
                break;
            case 0x88:
                //register     /memory to / from register
            case        0x89:
            case        0x8a:
            case        0x8b:
                op.        opsize = 2;
                get_operation(&op);
                op.d = get_bit(7, 7, op.byte[0]);
                //00000010 & byte
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                strcpy(op.opcode, "mov");
                determine_mod(&op);
                determine_reg(&op, 0);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
            case 0x8c:
            case 0x8d:
                op.opsize = 2;
                get_operation(&op);
                op.d = 1;
                op.w = 1;
                op.mod = get_bit(1, 2, op.byte[1]);
                op.reg = get_bit(3, 5, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                strcpy(op.opcode, "lea");
                determine_reg(&op, 0);
                determine_mod(&op);
                print_bynary(&op);
                print_operand(&op,opstr);
                break;
            case 0x8e:
            case 0x8f:
                break;
            case 0x90:
                //xchg Register with Accumulator
            case 0x91:
            case 0x92:
            case 0x93:
            case 0x94:
            case 0x95:
            case 0x96:
            case 0x97:
                op.opsize = 1;
                strcpy(op.opcode, "xchg");
                op.reg = get_bit(6, 8, op.byte[0]);
                op.w = 1;
                //
                determine_reg(&op, 0);
                print_bynary(&op);
                sprintf(opstr, "%s %s, ax", op.opcode, op.operand[0]);
                break;
            case 0x98:
                //cbw
                op.opsize = 1;
                strcpy(op.opcode, "cbw");
                print_bynary(&op);
                sprintf(opstr, "%s", op.opcode);
                break;
            case 0x99:
                op.opsize = 1;
                strcpy(op.opcode, "cwd");
                print_bynary(&op);
                sprintf(opstr, "%s", op.opcode);
                break;
            case 0x9a:
            case 0x9b:
            case 0x9c:
            case 0x9d:
            case 0x9e:
            case 0x9f:
                break;
            case 0xa0:
                //mov mem to acc
            case 0xa1:
            case 0xa2:
                op.opsize = 3;
                strcpy(op.opcode, "mov");
                get_operation(&op);
                op.w = get_bit(8, 8, op.byte[0]);
                print_bynary(&op);
                if (op.w == 1)
                    sprintf(opstr, "%s ax, [%02x%02x]", op.opcode, op.byte[2], op.byte[1]);
                else
                    sprintf(opstr, "%s al, [%02x%02x]", op.opcode, op.byte[2], op.byte[1]);
                op.r_m = -1;
                op.disp[1] = op.byte[2];
                op.disp[0] = op.byte[1];
                break;
            case 0xa3:
                //mov acc to mem
            case 0xa4:
                op.opsize = 3;
                strcpy(op.opcode, "mov");
                get_operation(&op);
                op.w = get_bit(8, 8, op.byte[0]);
                print_bynary(&op);
                if (op.w == 1)
                    sprintf(opstr, "%s [%02x%02x], ax", op.opcode, op.byte[2], op.byte[1]);
                else
                    sprintf(opstr, "%s [%02x%02x], al", op.opcode, op.byte[2], op.byte[1]);
                op.r_m = -1;
                op.disp[1] = op.byte[2];
                op.disp[0] = op.byte[1];
                break;
            case 0xa5:
                //cmps
            case 0xa6:
                op.opsize = 1;
                get_operation(&op);
                op.w = get_bit(8, 8, op.byte[0]);
                print_bynary(&op);
                if (op.w == 1)
                    strcpy(op.opcode, "cmpsw");
                else
                    strcpy(op.opcode, "cmpsb");
                sprintf(opstr, "%s", op.opcode);
            case 0xa7:
                break;
            case 0xa8:
                //test Immediate Data and Accumulator
            case 0xa9:
                strcpy(op.opcode, "test");
                op.w = get_bit(8, 8, op.byte[0]);
                op.reg = 0;
                //ax / al
                determine_reg(&op, 0);
                if (op.w == 1) {
                    op.opsize = 3;
                    get_operation(&op);
                    print_bynary(&op);
                    sprintf(opstr, "%s %s, %02x%02x", op.opcode, op.operand[0], op.byte[2], op.byte[1]);
                } else {
                    op.opsize = 2;
                    get_operation(&op);
                    print_bynary(&op);
                    sprintf(opstr, "%s %s, %x", op.opcode, op.operand[0], op.byte[1]);
                    //byte ? ??らない？mmvm ? ??はbyte ? ??表記ない
                }
                break;
            case 0xaa:
            case 0xab:
            case 0xac:
            case 0xad:
            case 0xae:
            case 0xaf:
                break;
            case 0xb0:
                //mov immediate to refister
            case 0xb1:
            case 0xb2:
            case 0xb3:
            case 0xb4:
            case 0xb5:
            case 0xb6:
            case 0xb7:
            case 0xb8:
            case 0xb9:
            case 0xba:
            case 0xbb:
            case 0xbc:
            case 0xbd:
            case 0xbe:
            case 0xbf:
                strcpy(op.opcode, "mov");
                op.w = get_bit(5, 5, op.byte[0]);
                op.reg = get_bit(6, 8, op.byte[0]);
                if (op.w == 1) {
                    op.opsize = 3;
                    get_operation(&op);
                    determine_reg(&op, 0);
                    print_bynary(&op);
                    op.data[0] = op.byte[1];
                    op.data[1] = op.byte[2];
                    sprintf(opstr, "%s %s, %02x%02x", op.opcode, op.operand[0], op.data[1], op.data[0]);
                } else {
                    op.opsize = 2;
                    get_operation(&op);
                    determine_reg(&op, 0);
                    print_bynary(&op);
                    sprintf(opstr, "%s byte %s, %x", op.opcode, op.operand[0], op.byte[1]);
                }
                
                break;
            case 0xc0:
            case 0xc1:
            case 0xc2:
                //ret Within Seg Adding Immed to SP
                op.opsize = 3;
                strcpy(op.opcode, "ret");
                get_operation(&op);
                print_bynary(&op);
                op.data[0] = op.byte[1];
                op.data[1] = op.byte[2];
                //インタプリタ用
                sprintf(opstr, "%s %02x%02x", op.opcode, op.byte[2], op.byte[1]);
                break;
            case 0xc3:
                op.opsize = 1;
                strcpy(op.opcode, "ret");
                print_bynary(&op);
                sprintf(opstr, "%s", op.opcode);
                break;
            case 0xc4:
            case 0xc5:
                break;
            case 0xc6:
                //immediate to register /memory
            case        0xc7:
                op.        opsize = 2;
                strcpy(op.opcode, "mov");
                get_operation(&op);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                determine_mod(&op);
                op.opsize++;
                op.data[0] = op.byte[op.opsize - 1] = get_text();
                switch (get_bit(3, 5, op.byte[1])) {
                    case 0:
                        if (op.w == 1) {
                            op.opsize++;
                            op.byte[op.opsize - 1] = op.data[1] = get_text();
                            print_bynary(&op);
                            if (op.data[1] == 0)
                                //表示を合わせるための分岐
                                sprintf(opstr, "%s %s, %04x", op.opcode, op.operand[1], op.data[0]);
                            else
                                sprintf(opstr, "%s %s, %02x%02x", op.opcode, op.operand[1], op.data[1], op.data[0]);
                        } else {
                            print_bynary(&op);
                            sprintf(opstr, "%s byte %s, %x", op.opcode, op.operand[1], op.data[0]);
                        }
                        break;
                    default:
                        sprintf(opstr, "defaultです\n");
                        break;
                }
                break;
            case 0xc8:
            case 0xc9:
            case 0xca:
            case 0xcb:
            case 0xcc:
                break;
            case 0xcd:
                op.opsize = 2;
                strcpy(op.opcode, "int");
                get_operation(&op);
                print_bynary(&op);
                op.data[0] = op.byte[1];
                sprintf(opstr, "%s %x", op.opcode, op.data[0]);
                break;
            case 0xce:
            case 0xcf:
                break;
            case 0xd0:
                //shl shr rcl sar
            case 0xd1:
            case 0xd2:
            case 0xd3:
                op.opsize = 2;
                get_operation(&op);
                op.v = get_bit(7, 7, op.byte[0]);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                switch (get_bit(3, 5, op.byte[1])) {
                    case 2:
                        strcpy(op.opcode, "rcl");
                        break;
                    case 4:
                        strcpy(op.opcode, "shl");
                        break;
                    case 5:
                        strcpy(op.opcode, "shr");
                        break;
                    case 7:
                        strcpy(op.opcode, "sar");
                        break;
                    default:
                        sprintf(opstr, "defaultです\n");
                        break;
                        
                }
                int        IP1 = 0;
                determine_mod(&op);
                print_bynary(&op);
                if (op.v != 1) {
                    IP1 = 1;
                    op.data[0] = IP1;
                    sprintf(opstr, "%s %s, %x", op.opcode, op.operand[1], IP1);
                } else {
                    //IP in cl
                    sprintf(opstr, "%s %s, cl", op.opcode, op.operand[1]);
                }
                
                break;
            case 0xd4:
            case 0xd5:
            case 0xd6:
            case 0xd7:
            case 0xd8:
            case 0xd9:
            case 0xda:
            case 0xdb:
            case 0xdc:
            case 0xdd:
            case 0xde:
            case 0xdf:
            case 0xe0:
                op.opsize = 2;
                get_operation(&op);
                strcpy(op.opcode, "loopnz");
                print_bynary(&op);
                if (get_bit(1, 1, op.byte[1]) == 1) {
                    op.byte[1] = -(0x100 - op.byte[1]);
                    //補数
                }
                sprintf(opstr, "%s %04x", op.opcode, IP - 0x20 + op.byte[1]);
                break;
            case 0xe1:
                op.opsize = 2;
                get_operation(&op);
                strcpy(op.opcode, "loopz");
                print_bynary(&op);
                if (get_bit(1, 1, op.byte[1]) == 1) {
                    op.byte[1] = -(0x100 - op.byte[1]);
                    //補数
                }
                sprintf(opstr, "%s %04x", op.opcode, IP - 0x20 + op.byte[1]);
                break;
            case 0xe2:
                op.opsize = 2;
                get_operation(&op);
                strcpy(op.opcode, "loop");
                print_bynary(&op);
                int        looptemp = op.byte[1];
                op.data[0] = op.byte[1];
                //インタプリタ用
                if (get_bit(1, 1, op.byte[1]) == 1) {
                    looptemp = -(0x100 - op.byte[1]);
                    //補数
                }
                sprintf(opstr, "%s %04x", op.opcode, (looptemp + IP));
                op.data[0] = looptemp + IP;
                op.data[1] = (looptemp + IP) >> 8;
                break;
            case 0xe3:
                op.opsize = 2;
                get_operation(&op);
                strcpy(op.opcode, "jcxz");
                print_bynary(&op);
                if (get_bit(1, 1, op.byte[1]) == 1) {
                    op.byte[1] = -(0x100 - op.byte[1]);
                    //補数
                }
                sprintf(opstr, "%s %04x", op.opcode, IP - 0x20 + op.byte[1]);
                break;
            case 0xe4:
            case 0xe5:
                op.opsize = 2;
                strcpy(op.opcode, "in");
                get_operation(&op);
                op.w = get_bit(8, 8, op.byte[0]);
                op.reg = 0;
                //ax ? ??al ? ??かない？後で調べる
                determine_reg(&op, 0);
                print_bynary(&op);
                sprintf(opstr, "%s %s, %x", op.opcode, op.operand[0], op.byte[1]);
                break;
            case 0xe6:
            case 0xe7:
                /*
                 * op.opsize=2; get_operation(&op);
                 * op.w=get_bit(8,8,op.byte[0]);
                 * op.mod=get_bit(1,2,op.byte[1]);
                 * op.r_m=get_bit(6,8,op.byte[1]);
                 * determine_mod(&op);
                 * switch(get_bit(3,5,op.byte[1])){ case 4:
                 * strcpy(op.opcode,"mul");break; }
                 */
                
                break;
            case 0xe8:
                //call direct within segment
                op.w = 1;
                //インタープリタ分岐用
                op.opsize = 3;
                strcpy(op.opcode, "call");
                get_operation(&op);
                print_bynary(&op);
                op.data[0] = op.byte[1];
                op.data[1] = op.byte[2];
                //check_operand ? ??岐のため
                //sprintf(opstr, "%s %04x", op.opcode, temp1);
                int        calltemp = (op.byte[2] << 8) + op.byte[1] + IP;
                calltemp &= 0xffff;
                sprintf(opstr, "%s %04x", op.opcode, calltemp);
                
                //インタプリたようした
                op.data[0] = calltemp;
                op.data[1] = calltemp >> 8;
                break;
            case 0xe9:
                //jmp direct within segment
                op.opsize = 3;
                op.w = 1;
                //インタープリターの分岐用w ? ??操作なく2 byte ? ??み込みだから??231応
                strcpy(op.opcode, "jmp");
                get_operation(&op);
                print_bynary(&op);
                op.data[0] = op.byte[1];
                op.data[1] = op.byte[2];
                //インタープリターのcheck_operand ? ??分岐のため入れる
                //sprintf(opstr, "%s %04x", op.opcode, (op.byte[1] + op.byte[2] * 0x100 + IP - 0x20) % 0x10000);
                int        jmptemp = op.data[0] + (op.data[1] << 8) + IP;
                sprintf(opstr, "%s %04x", op.opcode, jmptemp & 0xffff);
                //インタプリタ用
                op.data[0] = jmptemp;
                op.data[1] = jmptemp >> 8;
                break;
            case 0xea:
            case 0xeb:
                //jump short
                op.        opsize = 2;
                strcpy(op.opcode, "jmp short");
                get_operation(&op);
                print_bynary(&op);
                int        jmpshorttemp = op.byte[1];
                op.data[0] = op.byte[1];
                //インタプリタ用
                if (get_bit(1, 1, op.byte[1]) == 1) {
                    jmpshorttemp = -(0x100 - op.byte[1]);
                    //補数
                }
                sprintf(opstr, "%s %04x", op.opcode, (jmpshorttemp + IP));
                op.data[0] = jmpshorttemp + IP;
                op.data[1] = (jmpshorttemp + IP) >> 8;
                break;
            case 0xec:
                //in variable port ? 231231?231231?231231ax or al ? ??dx ? ??指定先を入力する命令？
            case 0xed:
                op.opsize = 1;
                strcpy(op.opcode, "in");
                op.w = get_bit(8, 8, op.byte[0]);
                op.reg = 0;
                //ax or al ? 231231後で確認
                determine_reg(&op, 0);
                print_bynary(&op);
                sprintf(opstr, "%s %s, dx", op.opcode, op.operand[0]);
                //テスト用バイナリで試したら全部dx ? ??らax oe al ? ??った
                break;
            case 0xee:
            case 0xef:
            case 0xf0:
            case 0xf1:
                break;
            case 0xf2:
            case 0xf3:
                op.z = get_bit(8, 8, op.byte[0]);
                op.opsize = 2;
                strcpy(op.opcode, "rep");
                get_operation(&op);
                print_bynary(&op);
                //sprintf(opstr, "%s （%02xで始まる命令)", op.opcode, op.byte[1]);
                char        str       [10];
                if (op.byte[0] == 0xf3)
                    switch (op.byte[1]) {
                        case 0x6c:
                            strcmp(str, "insb");
                            break;
                        case 0x6d:
                            strcpy(str, "insw");
                            break;
                        case 0xa4:
                            strcmp(str, "movsb");
                            break;
                        case 0xa5:
                            strcpy(str, "movsw");
                            break;
                        case 0x6e:
                            strcpy(str, "outsb");
                            break;
                        case 0x6f:
                            strcpy(str, "outsw");
                            break;
                        case 0xac:
                            strcpy(str, "lodsb");
                            break;
                        case 0xad:
                            strcpy(str, "lodsw");
                            break;
                        case 0xaa:
                            strcpy(str, "stosb");
                            break;
                        case 0xab:
                            strcpy(str, "stosw");
                            break;
                        case 0xa6:
                            strcpy(str, "cmpsb");
                            break;
                        case 0xa7:
                            strcpy(str, "cmpsw");
                            break;
                        case 0xae:
                            strcpy(str, "scasb");
                            break;
                        case 0xaf:
                            strcpy(str, "scasw");
                            break;
                        default:
                            fprintf(stderr, "reperror");
                            break;
                    }
                else
                    switch (op.byte[1]) {
                        case 0xa4:
                            //仕様書とminix ? ??実装が違う？f3a4 ? ??なくf2a4 ? ??movsb
                            strcpy(str, "movsb");
                            break;
                        case 0xa5:
                            strcpy(str, "movsw");
                            break;
                        case 0xa6:
                            strcpy(str, "cmpsb");
                            break;
                        case 0xa7:
                            strcpy(str, "cmpsw");
                            break;
                        case 0xaa:
                            strcpy(str, "stosb");
                            break;
                        case 0xab:
                            strcpy(str, "stosw");
                            break;
                        case 0xae:
                            strcpy(str, "scasb");
                            break;
                        case 0xaf:
                            strcpy(str, "scasb");
                            break;
                        default:
                            fprintf(stderr, "reperror");
                    }
                sprintf(opstr, "%s %s", op.opcode, str);
                break;
            case 0xf4:
                //hlt
                op.opsize = 1;
                strcpy(op.opcode, "hlt");
                print_bynary(&op);
                sprintf(opstr, "%s", op.opcode);
            case 0xf5:
                break;
            case 0xf6:
                //neg && test immediate data and r / m
            case 0xf7:
                op.opsize = 2;
                get_operation(&op);
                op.w = get_bit(8, 8, op.byte[0]);
                op.mod = get_bit(1, 2, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                determine_mod(&op);
                switch (get_bit(3, 5, op.byte[1])) {
                    case 0:
                        strcpy(op.opcode, "test");
                        op.opsize++;
                        op.byte[op.opsize - 1] = op.data[0] = get_text();
                        if (op.w == 1) {
                            op.opsize++;
                            op.byte[op.opsize - 1] = op.data[1] = get_text();
                            print_bynary(&op);
                            //if (op.data[1] == 0)
                            //表示を合わせるための分岐
                            //sprintf(opstr, "%s %s, %04x\n", op.opcode, op.operand[1], op.data[0]);
                            //
                            sprintf(opstr, "%s %s, %02x%02x", op.opcode, op.operand[1], op.data[1], op.data[0]);
                        } else {
                            print_bynary(&op);
                            if (op.mod != 3)
                                //この条件分岐はとりあえず表記を合わせるための??399時的なもの?399399あとで改善する
                                sprintf(opstr, "%s byte %s, %x", op.opcode, op.operand[1], op.data[0]);
                            else
                                //これも
                                sprintf(opstr, "%s %s, %x", op.opcode, op.operand[1], op.data[0]);
                        }
                        break;
                    case 3:
                        strcpy(op.opcode, "neg");
                        print_bynary(&op);
                        sprintf(opstr, "%s %s", op.opcode, op.operand[1]);
                        break;
                    case 4:
                        strcpy(op.opcode, "mul");
                        print_bynary(&op);
                        sprintf(opstr, "%s %s", op.opcode, op.operand[1]);
                        break;
                    case 6:
                        strcpy(op.opcode, "div");
                        print_bynary(&op);
                        sprintf(opstr, "%s %s", op.opcode, op.operand[1]);
                        break;
                    case 7:
                        strcpy(op.opcode, "idiv");
                        print_bynary(&op);
                        sprintf(opstr, "%s %s", op.opcode, op.operand[1]);
                        break;
                        
                    default:
                        sprintf(opstr, "default\n");
                }
                break;
            case 0xf8:
            case 0xf9:
            case 0xfa:
            case 0xfb:
                break;
            case 0xfc:
                //cld
                op.opsize = 1;
                strcpy(op.opcode, "cld");
                print_bynary(&op);
                sprintf(opstr, "%s", op.opcode);
                break;
            case 0xfd:
                op.opsize = 1;
                strcpy(op.opcode, "std");
                print_bynary(&op);
                sprintf(opstr, "%s", op.opcode);
                break;
            case 0xfe:
                break;
            case 0xff:
                //call indirect within segment
                op.opsize = 2;
                get_operation(&op);
                op.w = 1;
                //
                op.mod = get_bit(1, 2, op.byte[1]);
                op.r_m = get_bit(6, 8, op.byte[1]);
                switch (get_bit(3, 5, op.byte[1])) {
                    case 0:
                        strcpy(op.opcode, "inc");
                        break;
                    case 1:
                        strcpy(op.opcode, "dec");
                        break;
                    case 2:
                        strcpy(op.opcode, "call");
                        break;
                    case 4:
                        strcpy(op.opcode, "jmp");
                        break;
                    case 6:
                        strcpy(op.opcode, "push");
                        break;
                    default:
                        sprintf(opstr, "defaultです\n");
                        break;
                }
                
                determine_mod(&op);
                print_bynary(&op);
                sprintf(opstr, "%s %s", op.opcode, op.operand[1]);
                break;
        }
        fprintf(stderr, "%s", opstr);
        for (int i = 0; i < 4; i++) {
            op.asem[i] = "";
        }
        split(opstr, ", ", op.asem);
        if (!strcmp(op.asem[1], "byte")) {
            op.asem[1] = op.asem[2];
            op.asem[2] = op.asem[3];
        }
        exec(&op);
        strcpy(opcode, op.opcode);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\ntsize<IP ! tsize=%04x, ip = %04x\n",tsize,IP);
    destroy_data(mem);
    return 0;
}
