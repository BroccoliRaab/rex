#define REX_IMPLEMENTATION
#include "rex.h"

#include <stdlib.h>

int main()
{
    rex_compiler_t rexc;
    rex_instruction_t prog[4096];
    uint8_t mem[1024];
    REX_SIZE_T prog_sz;
    const unsigned char regex[5] = "a+b+";
    int r;
    rexc.mem = mem;
    rexc.mem_sz = 1024;
    r = rex_compile_regex(
        regex, 5,
        NULL, &prog_sz,
        &rexc
    );
    r = rex_compile_regex(
        regex, 5,
        prog, &prog_sz,
        &rexc
    );
    if (r) return r;

    return r;
}
