#define REX_IMPLEMENTATION
#include "rex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char ** argv)
{
    rex_compiler_t rexc;
    uint8_t mem[1024];
    unsigned char ast_str[512];
    rex_instruction_t prog[128];
    int r;
    rexc.mem = mem;
    rexc.mem_sz = 1024;

    r= rex_ast_build(
        ((unsigned char **)argv)[1],
        strlen(argv[1]),
        &rexc
    );
    if (r) return r;

    rex_ast_tostring(
        &rexc,
        ast_str
    );
    fputs((const char * )ast_str, stdout);
    r = rex_ast_compile(
        &rexc,
        0,
        prog
    );
    if (r) return r;

    return r;
}
