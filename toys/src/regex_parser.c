#define REX_IMPLEMENTATION
#include "rex.h"
#include <stdio.h>

#define TOKEN_CASE(token) \
    case token: \
    puts(#token); data++; break;

int
main(void)
{
    char line[512];
    uint8_t mem[1024];
    rex_compiler_t c;
    int r;
    uint8_t * data;
    size_t l;
    c.memory = mem;
    c.memory_sz = 1024;
    for(;;)
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
        for (data = c.ast_top; data < mem + 1024;)
        {
            switch(*data)
            {
                REX_TOKEN_X(TOKEN_CASE);
                default:
                l = rex_parse_charset((char *)data,SIZE_MAX);
                if (!l) break;
                fwrite(
                    data,
                    1,
                    l,
                    stdout
                );
                data+=l;
                putchar('\n');
                break;
            }
        }
        putchar('\n');
    }
}
