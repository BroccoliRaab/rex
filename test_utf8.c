#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define REX_IMPLEMENTATION
#include "rex.h"


int main(void)
{
    char line[8] = "";
    size_t len;
    uint32_t cp;

    do{
        fgets(line, 8, stdin);
        len = rex_parse_utf8_codepoint(line, 8, &cp);
        printf("%s : U+%x LEN: %ld\n", line, cp, len); 

    }while (line[0] != '\n');
}




