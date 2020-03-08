#include "vmiss.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/un.h>
#include<unistd.h>
#include <netinet/in.h>
#include "json_rsc.h"

int compare_vm(char* sendstr,VM* student,VM* serv){
    int equals = 0;
    if (student->mutable.gregs.r16[0] != serv->mutable.gregs.r16[0]) {
        resource_json(sendstr, "AX",serv->mutable.gregs.r16[0],student->mutable.gregs.r16[0]);
        fprintf(stderr, "AXが異なります");
        equals = 1;
    }
    if (student->mutable.gregs.r16[1] != serv->mutable.gregs.r16[1]) {
        resource_json(sendstr, "BX",serv->mutable.gregs.r16[1],student->mutable.gregs.r16[1]);
        fprintf(stderr, "BXが異なります");
        equals = 1;
    }
    if (student->mutable.gregs.r16[2] != serv->mutable.gregs.r16[2]) {
        resource_json(sendstr, "CX",serv->mutable.gregs.r16[2],student->mutable.gregs.r16[2]);
        fprintf(stderr, "CXが異なります");
        equals = 1;
    }
    if (student->mutable.gregs.r16[3] != serv->mutable.gregs.r16[3]) {
        resource_json(sendstr, "DX",serv->mutable.gregs.r16[3],student->mutable.gregs.r16[3]);
        fprintf(stderr, "DXが異なります");
        equals = 1;
    }
    if(student->mutable.regs[0] != serv->mutable.regs[0]) {
        resource_json(sendstr, "SP", serv->mutable.regs[0], student->mutable.regs[0]);
        fprintf(stderr, "SPが異なります");
        equals = 1;
    }
    if (student->mutable.regs[1] != serv->mutable.regs[1]) {
        resource_json(sendstr, "BP", serv->mutable.regs[1], student->mutable.regs[1]);
        fprintf(stderr, "BPが異なります");
        equals = 1;
    }
    if (student->mutable.regs[2] != serv->mutable.regs[2]) {
        resource_json(sendstr, "SI", serv->mutable.regs[2], student->mutable.regs[2]);
        fprintf(stderr, "SIが異なります");
        equals = 1;
    }
    if (student->mutable.regs[3] != serv->mutable.regs[3]) {
        resource_json(sendstr, "DI", serv->mutable.regs[3], student->mutable.regs[3]);
        fprintf(stderr, "DIが異なります");
        equals = 1;
    }
    if (student->mutable.ip!= serv->mutable.ip) {
        resource_json(sendstr, "IP", serv->mutable.ip, student->mutable.ip);
        fprintf(stderr, "IPが異なります");
        equals = 1;
    }
    if (student->mutable.flags != serv->mutable.flags) {
        unsigned short cflg = student->mutable.flags;
        unsigned short sflg = serv->mutable.flags;
        if (( (cflg >> OF_BIT) & 1 ) != ( (sflg >> OF_BIT) & 1) )
            resource_json(sendstr, "OF", ((sflg >> OF_BIT) & 1 ), ((cflg >> OF_BIT) & 1));
        if (( (cflg >> ZF_BIT) & 1 ) != ( (sflg >> ZF_BIT) & 1) )
            resource_json(sendstr, "ZF", ((sflg >> ZF_BIT) & 1 ), ((cflg >> ZF_BIT) & 1));
        if (( (cflg >> SF_BIT) & 1 ) != ( (sflg >> SF_BIT) & 1) )
            resource_json(sendstr, "SF", ((sflg >> SF_BIT) & 1 ), ((cflg >> SF_BIT) & 1));
        if (( (cflg >> CF_BIT) & 1 ) != ( (sflg >> CF_BIT) & 1) )
            resource_json(sendstr, "CF", ((sflg >> CF_BIT) & 1 ), ((cflg >> CF_BIT) & 1));
        equals = 1;
    }
    if (memcmp(student->mutable.data_mem, serv->mutable.data_mem, 0x10000) != 0) {
        if (fdflag == 1) {
            fd_tbl[student->mutable.data_mem[BX + 2]] = serv->mutable.data_mem[BX + 2];
            dmem[BX + 2] = serv->mutable.data_mem[BX + 2] = student->mutable.data_mem[BX + 2];
            fdflag = 0;
            if (memcmp(student->mutable.data_mem, serv->mutable.data_mem, 0x10000) != 0) {
                equals = 1;
            }
        } else {
            equals = 1;
        }
    }
    
    if (equals == 1) {
        for (int i = 0; i < 0x10000; i++) {
            if (student->mutable.data_mem[i] != serv->mutable.data_mem[i]) {
                char str[64] = "";
                sprintf(str, "memory[%04x]", i); 
                resource_json(sendstr, str, serv->mutable.data_mem[i] ,student->mutable.data_mem[i] );
                //fprintf(stderr, "datamem[%04x]{server:%02x,client:%02x}\n", i, serv->mutable.data_mem[i], student->mutable.data_mem[i]);
            }
        }
    }
    return equals;
}

int send_all(int fd, void *ptr, int len) {
    int total = 0;
    while (total < len) {
        int result = (int) write(fd, (char *)ptr + total, len - total);
        if (result == -1){
            perror("write");
            return -1;
        }
        else {
            total += result;
        }
    }
    return total;
}

int recv_all(int fd, void *ptr, int len) {
    int total = 0;
    while (total < len) {
        int result = (int) read(fd, (char *) ptr + total, len - total);
        if (result == -1) {
            perror("read");
            return result;
        }
        total += result;
    }
    return total;
}


int compare(int fd, VM* serv_vm, char* opcode){
    VM recv_vm = {0};
    recv_all(fd, &recv_vm, sizeof(recv_vm.mutable));
    char sendstr[0x100000] = {}; 
    //json
    strcat(sendstr,"{");
    instruction_json(sendstr, opcode);
    strcat(sendstr,",\"resource\":[");
    //json
    
    if (compare_vm(sendstr,&recv_vm, serv_vm)) {
        /*unsigned long len = strlen(sendstr);
         write(fd,&len, sizeof(len));
         write(fd, sendstr, len);*/
        char judge_result = 1;
        send_all(fd,&judge_result,1);
        
        //json
        sendstr[strlen(sendstr) - 2] = ']';
        sendstr[strlen(sendstr) - 1] = '}';
        //json
        
        fprintf(stderr,"%s", sendstr);
        
        unsigned long user_id = 0;//sample
        char filename[32];
        sprintf(filename,"out/%08lx.json",user_id);
        FILE *fp = fopen(filename,"w");
        fwrite(sendstr, sizeof(char),strlen(sendstr),fp);
        fclose(fp);
        /*
         
         unsigned long len = strlen(sendstr);
         send_all(fd,&len,sizeof len);
         send_all(fd, sendstr,(int)len);
         //debug_hw(servhw);
         //debug_hw(recvhw);
        */
        close(fd);
        exit(0);
    }else{
        send_all(fd, "", 1);
    }
    return 1;
}

int wait_client(){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    int one = 1;
    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &one, sizeof(one)) < 0) {
        perror("ERROR on setsockopt");
        exit(1);
    }
    
    if (serv_sock == -1) {
        perror("socket");
        return 1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8888);
    
    if (bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof serv_addr) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(serv_sock, 5) == -1) {
        perror("listen");
        exit(1);
    }
    socklen_t clntlen = sizeof clnt_addr;
    clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clntlen);
    if (clnt_sock == -1) {
        perror("accept");
        exit(1);
    }
    return clnt_sock;
}

