#include <stdint.h>
#include <stddef.h>

#include <stdio.h>

#define REX_CLZ8(i) __builtin_clz((((unsigned int)i)<<24) | 1)

static inline size_t
rex_parse_utf8_codepoint(
    const char * const i_str,
    size_t i_len,
    uint32_t * const o_cp
);

static inline size_t
rex_parse_utf8_codepoint(
    const char * const i_str,
    size_t i_len,
    uint32_t * const o_cp
){
    uint8_t b;
    uint8_t lo;
    if (!(i_len && i_str)) return 0;
    b = *i_str;
    lo = REX_CLZ8(~b); 
    printf("\n aaaa %d aaaa\n", lo);
    if (lo > i_len) return 0;
    *o_cp = 0;
    switch(lo)
    {
    case 0:
        *o_cp = b;
        return 1;
    case 2:
        *o_cp = (b & 0b00011111) << 6;
        *o_cp |= i_str[1] & 0b00111111;
        return 2;
    case 3:
        *o_cp = (b & 0b00001111) << 12;
        *o_cp |= (i_str[1] & 0b00111111) << 6;
        *o_cp |= i_str[2] & 0b00111111;
        return 3;
    case 4:
        *o_cp = (b & 0b00000111) << 18;
        *o_cp |= (i_str[1] & 0b00111111) << 12;
        *o_cp |= (i_str[2] & 0b00111111) << 6;
        *o_cp |= i_str[3] & 0b00111111;
        return 4;
    default:
        return 0;
    }
}


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




