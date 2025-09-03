#define REX_IMPLEMENTATION
#include "rex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* 
 * current implementation infinitely loops on
 * [\d\D]
 */
#define REX_MAX(start0, start1) \
    (((start0) > (start1))? (start0) : (start1))
#define REX_MIN(start0, start1) \
    (((start0) < (start1))? (start0) : (start1))
#define REX_INTERSECTION_EXISTS(r0_start, r0_end, r1_start, r1_end) \
    (REX_MAX(r0_start, r1_start) <= REX_MIN(r0_end, r1_end))
#define REX_MIN_INTERSECTION_START(min0, min1, cur0, cur1, super0, super1)  \
    (REX_INTERSECTION_EXISTS(cur0, cur1, super0, super1) ?                  \
        REX_MIN(min0, cur0) : min0)
#define REX_MIN_INTERSECTION_END(min0, min1, cur0, cur1, super0, super1)  \
    (REX_INTERSECTION_EXISTS(cur0, cur1, super0, super1) ?                \
        ((min0 < cur0) ? min1 : cur1) : min1)

static inline void rex_mcset_body_range_sort(
    const char * const i_body,
    uint32_t * const io_r0,
    uint32_t * const io_r1
)
{
    size_t l, i, ri, rsz;
    uint32_t cp0, cp1;
    uint32_t min_cp0, min_cp1;
    min_cp0 = REX_MAX_UNICODE_VAL + 1;
    min_cp1 = REX_MAX_UNICODE_VAL + 1;
    for (
        l = 0, i = 0, ri =0;
        i_body[l] != ']';
        l+=i,
        min_cp0 = REX_MIN_INTERSECTION_START(
            min_cp0, min_cp1, cp0, cp1, *io_r0, *io_r1),
        min_cp1 = REX_MIN_INTERSECTION_END(
            min_cp0, min_cp1, cp0, cp1, *io_r0, *io_r1)
    )
    {
        i = rex_parse_multichar_range(i_body + l, SIZE_MAX, &cp0, &cp1);
        if (i) continue;

        i = rex_parse_single_char(i_body + l, SIZE_MAX, &cp0);
        if(i){
            cp1 = cp0;
            continue;
        }

        i = rex_parse_multichar_escape(i_body + l, SIZE_MAX);
        if (i)
        {
            switch(i_body[l + 1])
            {
            case 'W':
                rsz = rex_W_range_set_sz;
                cp0 = rex_W_range_set[ri].r0;
                cp1 = rex_W_range_set[ri].r1;
                break;
            case 'w':
                rsz = rex_w_range_set_sz;
                cp0 = rex_w_range_set[ri].r0;
                cp1 = rex_w_range_set[ri].r1;
                break;
            case 'S':
                rsz = rex_S_range_set_sz;
                cp0 = rex_S_range_set[ri].r0;
                cp1 = rex_S_range_set[ri].r1;
                break;
            case 's':
                rsz = rex_s_range_set_sz;
                cp0 = rex_s_range_set[ri].r0;
                cp1 = rex_s_range_set[ri].r1;
                break;
            case 'D':
                rsz = rex_D_range_set_sz;
                cp0 = rex_D_range_set[ri].r0;
                cp1 = rex_D_range_set[ri].r1;
                break;
            case 'd':
                rsz = rex_d_range_set_sz;
                cp0 = rex_d_range_set[ri].r0;
                cp1 = rex_d_range_set[ri].r1;
                break;
            }
            ri++;
            i = (ri == rsz)? i : 0;
            ri = (ri == rsz)? 0 : ri;
            continue;
        }
    }
    *io_r0 = min_cp0;
    *io_r1 = min_cp1;
}


/* 
 * Takes a range [*io_r0, *io_r1] as input
 * Finds the least range that is a mutual subset of i_body and [*io_r0, *io_r1]
 * If no subset is found, *io_r0 == *io_r1 > CODEPOINT_MAX_VAL
 *
 * No validation performed against i_body
 * Inputting UINT32_MAX breaks this function
 */
static inline void rex_mcset_body_simplify(
    const char * const i_body,
    uint32_t * const io_r0,
    uint32_t * const io_r1
)
{
    size_t l, i;
    uint32_t cp0, cp1;
    uint32_t min_cp0, min_cp1;

    min_cp0 = *io_r0;
    min_cp1 = *io_r1;
    rex_mcset_body_range_sort(
        i_body,
        &min_cp0,
        &min_cp1
    );
    if (min_cp1 < REX_MAX_UNICODE_VAL) for(;;) {
        cp0 = min_cp1+1;
        cp1 = REX_MAX_UNICODE_VAL;
        rex_mcset_body_range_sort(
            i_body,
            &cp0,
            &cp1
        );
        if (cp0 > min_cp1+1) break;
        /* Merge ranges */
        min_cp1 = cp1;
    };
    *io_r0 = min_cp0;
    *io_r1 = min_cp1;
}
  

int main(int argc, char ** argv)
{
    rex_compiler_t rexc;
    uint8_t mem[1024];
    unsigned char re_str[512];
    size_t re_str_sz;
    size_t i;
    rex_instruction_t prog[128];
    int r;
    uint32_t cp0, cp1;

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
    if (re_str[1] == '^') continue;
    puts("As sorted Ranges:");
    
    cp0 = 0, cp1 = REX_MAX_UNICODE_VAL;
    for(;;) {
        rex_mcset_body_simplify(
            re_str+1,
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
    return 0;
}


