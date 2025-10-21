#define REX_IMPLEMENTATION
#include "rex.h"

typedef struct rex_submatch_s rex_submatch_t;
typedef struct rex_vm_thread_stack_s rex_vm_thread_stack_t;

struct rex_submatch_s
{
    char * const match;
    size_t match_sz;
};

/* sizes and capacities are in bytes */
struct rex_vm_thread_stack_s
{
    void * const top;
    size_t sz;
    size_t thread_sz;
    const size_t cap;
};

void *
rex_vm_thread_stack_iter(
    rex_vm_thread_stack_t * const io_thread_stack,
    const void * const i_thread
)
{
    const uint8_t * const top_arith = io_thread_stack->top;
    const uint8_t * const thread_arith = i_thread;
    const uint8_t * const bot = top_arith + io_thread_stack->sz;
    const uint8_t * const next = thread_arith + io_thread_stack->thread_sz;
    if (next >= bot) return NULL;
    return (void *) next;
}
//TODO: First pull assembler test ISA into rex.h

size_t 
rex_vm_thread(
    rex_submatch_t * i_sub,
    size_t i_sub_sz,
    void * o_thread
)
{
    if (!o_thread) return i_sub_sz * sizeof(rex_submatch_t) + sizeof(uint32_t);
    if (!i_sub) return 0;
    // TODO

}

//TODO collapse into thread expand
int rex_vm_thread_needs_expansion(
    rex_vm_thread_stack_t * const io_thread_stack,
    const rex_instruction_t * const i_prog,
    void * const i_thread
)
{
    //TODO
    return 0;
}

int
rex_vm_thread_expand(
    rex_vm_thread_stack_t * const io_thread_stack,
    const rex_instruction_t * const i_prog,
    void * const i_thread
){
    uint32_t * pc = i_thread;
    // TODO: Extract OPCODE
    switch(i_prog[*pc])
    {
        default:
            return 0;
        //JUMP
    }
}

void
rex_vm_add_thread(
    rex_vm_thread_stack_t * const io_thread_stack,
    const void * const i_thread,
    const size_t i_thread_sz,
    const char * const i_cp,
    const rex_instruction_t * const i_prog
)
{
    void * cur_thread;
    //TODO: Actually add the thread
    /*Expansion Phase */
    do
    {
        /* Find first expandable thread */
        for (
            cur_thread = io_thread_stack->sz > 0 ?
                io_thread_stack->top :
                NULL;
            cur_thread;
            cur_thread = rex_vm_thread_stack_iter(io_thread_stack, cur_thread)
        ){
            if (
                rex_vm_thread_needs_expansion(
                    io_thread_stack, 
                    i_prog, 
                    cur_thread
                )
            ){
                break;
            }
        }
        /* expand it */
        rex_vm_thread_expand(
            io_thread_stack, 
            i_prog, 
            cur_thread
        );
    }while(cur_thread);
}



int
match(
    const char * const i_string,
    const rex_instruction_t * const i_prog,
    rex_submatch_t * o_submatches,
    size_t * io_submatches_sz
)
{
   return 1; 
}


int main(void)
{

    return 0;
}
