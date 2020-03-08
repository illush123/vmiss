#include "util.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

#define MAXITEM 10

unsigned char* create_data_from_file(const char *filename) {
    FILE *fp;
    long filesize = get_read_file_size(filename);
    if((fp = fopen(filename, "rb")) == NULL){
        perror("open");
        exit(1);
        return NULL;
    }
    unsigned char *data = (unsigned char*)malloc(filesize + 1);
    memset(data, 0, filesize);
    fread(data, filesize, 1, fp);
    data[filesize] = '\0';
    fclose(fp);
    return data;
}

void destroy_data(unsigned char *data) {
    free(data);
    return ;
}

long get_read_file_size(const char *in_fname)
{
    FILE *fp;
    long file_size;
    struct stat stbuf;
    int fd;
    fd = open(in_fname, O_RDONLY);
    if (fd == -1)
        printf("error");
    fp = fdopen(fd, "rb");
    if (fp == NULL)
        printf("error");
    if (fstat(fd, &stbuf) == -1)
        printf("error");
    file_size = stbuf.st_size;
    if (fclose(fp) != 0)
        printf("error");
    return file_size;
}

int split( char *str, const char *delim, char *outlist[] ) {
    char    *tk;
    int     cnt = 0;
    
    tk = strtok( str, delim );
    while( tk != NULL && cnt < MAXITEM ) {
        outlist[cnt++] = tk;
        tk = strtok( NULL, delim );
    }
    return cnt;
}
