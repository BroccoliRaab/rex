#define REX_IMPLEMENTATION
#include "rex.h"
#include <stdio.h>

#define TOKEN_CASE(token) \
    case token: \
    puts(#token); data++; break;



int
main(void)
{
    char line[512] = "a";
    uint8_t mem[1024];
    rex_instruction_t prog[1024] = {0};
    rex_compiler_t c;
    int r;
    size_t i;
    c.memory = mem;
    c.memory_sz = 1024;
    for(;line[0];)
    {
        fgets(line, 512, stdin);
        memset(mem, 0, 1024);
        line[strlen(line)-1] = 0;
        r = rex_build_ast(
            line,
            512,
            &c
        );
        if (r){
            puts("Parse Failure");
            continue;
        }
        r = rex_ast_compile(
            &c,
            prog,
            1024
        );
        if (r){
            puts("Compile Failure");
            continue;
        }
        for (i=0; REX_OP_FROM_INST(prog[i]) != REX_OPCODE_M; i++)
        {
            switch(REX_OP_FROM_INST(prog[i])){
            #define REX_PRINT_INSTRUCTION(name, opcode) \
                case opcode: \
                printf("0x%08lx %-8s0x%08x\n", i, #name, REX_IMM_FROM_INST(prog[i])); \
                break;
            REX_ISA_X(REX_PRINT_INSTRUCTION);
            }
        }
        printf("0x%08lx M       0x%08x\n\n", i, REX_IMM_FROM_INST(prog[i]));
    }
}
