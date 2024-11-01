#ifndef REX_H
#define REX_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

enum rex_cstate_e 
{
    REX_CPSTR = 0,
    REX_TOKENIZE
};

typedef enum rex_cstate_e rex_cstate_t;
typedef struct rex_compiler_s rex_compiler_t;
typedef void * regex_t;
typedef uint8_t (*rex_compiler_state_f)(rex_compiler_t *); 

uint8_t rex_cstate_error(rex_compiler_t * rc);
uint8_t rex_cstate_cpstr(rex_compiler_t * rc);
uint8_t rex_cstate_tokenize(rex_compiler_t * rc);
uint8_t rex_cstate_postfix(rex_compiler_t * rc);
uint8_t rex_cstate_dclasses(rex_compiler_t * rc);
uint8_t rex_cstate_dtree(rex_compiler_t * rc);

static rex_compiler_state_f rex_compiler_states[] ={
    rex_cstate_cpstr,
    rex_cstate_tokenize
};

struct rex_compiler_s 
{
    uint8_t * stack;
    size_t stack_sz;

    rex_cstate_t state;
    size_t offset;
    const char * re_str;
    size_t re_str_sz;
    
    uint32_t ri;
};

uint8_t rex_cstate_cpstr(rex_compiler_t * rc)
{
    if (rc->stack_sz < rc->offset + rc->re_str_sz) return 1;
    memcpy(rc->stack + rc->offset, rc->re_str, rc->re_str_sz+1);
    rc->offset += rc->re_str_sz;
    rc->ri = 0;
    return 0;
}

uint8_t rex_cstate_tokenize(rex_compiler_t * rc)
{
        

}

regex_t
compile_re(
    const char * re_str,
    rex_compiler_t *compiler
);



#endif /* REX_H */
