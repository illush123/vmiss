#include <stdio.h>
#include "util.h"
#include "disassm.h"
#include "interpreter.h"
#include "vmiss.h"
#include "x86emu.h"

int main(int argc, char *argv[]) {
    if(argc == 1) {
        printf("how to use : vmiss [filename] [args...]");
        return 1;
    }
    const char *env = "PATH=/bin:/usr/bin";
    run(argc - 1, &argv[1], env);
    return 0;
}




