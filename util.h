#ifndef util_h
#define util_h

#include <stdio.h>
unsigned char *create_data_from_file(const char *filename);
void destroy_data(unsigned char *data);
long get_read_file_size(const char *in_fname);
int split(char *str,const char *delim,char *outlist[]);
#endif /* util_h */
