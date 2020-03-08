#include "json_rsc.h"

void instruction_json(char* allstr, char *opcode){
    char str[64] = "";
    sprintf(str,"\"instruction\": \"%s\"",opcode);
    strcat(allstr,str);
}

void resource_json(char* allstr, char* name, unsigned short vmiss, unsigned short student){
  char str[128];
  sprintf(str, "{\"name\": \"%s\", \"vmiss\": %d, \"student\": %d}, ", name, vmiss, student);
  strcat(allstr, str);
}
