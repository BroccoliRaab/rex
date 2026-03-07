#define REX_IMPLEMENTATION
#include "rex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_ranges(
    char * re_str
)
{
    uint32_t cp0, cp1;    
    cp0 = 0, cp1 = REX_MAX_UNICODE_VAL;
    for(;;) {
        rex_mcset_body_simplify(
            re_str,
            &cp0,
            &cp1
        );
        if (cp0 > REX_MAX_UNICODE_VAL) break;
        printf("[%c, %c], ", cp0, cp1);
        cp0 = cp1 + 1;
        cp1 = REX_MAX_UNICODE_VAL;
    };
    puts("\n");
}

void print_ranges_inverted(
    char * re_str
)
{
    uint32_t cp0, cp1;    
    cp0 = 0, cp1 = REX_MAX_UNICODE_VAL;
    for(;;) {
        rex_mcset_body_simplify_inverted(
            re_str,
            &cp0,
            &cp1
        );
        if (cp0 > REX_MAX_UNICODE_VAL) break;
        printf("[%c, %c], ", cp0, cp1);
        cp0 = cp1 + 1;
        cp1 = REX_MAX_UNICODE_VAL;
    };
    puts("\n");
}

int main(int argc, char ** argv)
{
    rex_compiler_t rexc;
    uint8_t mem[1024];
    char re_str[512];
    size_t re_str_sz;
    size_t i;

    rexc.memory = mem;
    rexc.memory_sz = 1024;
    for (;;)
    {
        if (!fgets(re_str, 512, stdin))
        {
            continue;
        }
        
        re_str_sz = strlen(re_str);
        re_str[re_str_sz--] = 0;

        i = rex_parse_charset(
            re_str,
            re_str_sz+1
        );

        if (i == 0)
        {
            puts("Parse Failure");
            continue;
        } else if (i == re_str_sz)
        {
            puts("Parse Sucess");
        }else{
            puts("Trailing Characters");
            continue;
        }
        if (re_str[0] != '[') continue;
        puts("As sorted Ranges:");
        if (re_str[1] == '^')
        {
            print_ranges_inverted(re_str+2);
        }else
        {
            print_ranges(re_str+1);
        }

    }
    return 0;
}


