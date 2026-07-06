#define REX_IMPLEMENTATION
#include "rex.h"
#include <stdio.h>

#define TOKEN_CASE(token) \
    case token: \
    puts(#token); data++; break;


#define MEM_SZ 16384 
static uint8_t mem[MEM_SZ];

int
main(int argc, char ** argv)
{
    char line[512] = "a";
    rex_instruction_t prog[1024] = {0};
    rex_compiler_t c;
    rex_vm_t vm;
    int r;
    size_t i, sz;
    rex_match_t matches[16];
    int success;
    c.memory = mem;
    c.memory_sz = MEM_SZ;
    vm.memory = mem;
    vm.memory_sz = MEM_SZ;


    if (argc < 2) return 1;

    r = rex_build_ast(
        argv[1],
        512,
        &c
    );
    if (r){
        puts("Parse Failure");
        return r;
    }
    /* TODO: this needs to output a size*/
    /* TODO: this needs to output how man submatches it contained */
    r = rex_ast_compile(
        &c,
        prog,
        1024
    );
    if (r){
        puts("Compile Failure");
        return r;
    }

    for (sz = 0; prog[sz]; sz++);
    sz++;

    for(;line[0];)
    {
        memset(matches, 0, sizeof(matches));
        fgets(line, 512, stdin);
        memset(mem, 0, 1024);
        line[strlen(line)-1] = 0;
        r = rex_vm_exec(
            &vm,
            line,
            SIZE_MAX,
            0,
            prog,
            sz,
            matches,
            16,
            &success
        );
        if (r) {
            puts("Execution Failure");
            continue;
        }
        if (success && matches[0].match_sz == strlen(line))
        {
            puts("MATCH!\n");
            for (i = 0; matches[i].match; i++)
            {
                printf("[%lu] : ", i);
                fwrite(matches[i].match, 1, matches[i].match_sz, stdout);
                putchar('\n');
            }
        }else{
            puts("NOT A MATCH!");
        }
    }
}
