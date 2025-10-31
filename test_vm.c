#define REX_IMPLEMENTATION
#include "rex.h"

typedef struct rex_match_s rex_match_t;
typedef struct rex_vm_s rex_vm_t;
typedef struct rex_vm_threadlist_s rex_vm_threadlist_t;

/* TODO:
 * Save the match as submatch 0?
 * Makes the in vm search approach simpler by just inserting regex into .*?(regex)
 * 5x minimal thread size. 4 Bytes -> 20 Bytes
 *
 * Ideally want the minimal thread size and in vm search
 * Could do specific match start save instruction
 *
 */

/* Thread Memory Layout
 *
 * uint32_t : pc
 * const char *[2] : match start and end markers
 * const char *[N] : submatch markers
 *
 */

struct rex_vm_thread_s
{
    uint32_t pc;
};

struct rex_match_s
{
    char * const match;
    size_t match_sz;
};

struct rex_vm_s
{
    void * memory;
    size_t memory_sz;
};

struct rex_vm_threadlist_s
{
    void * buffer;
    size_t thread_count;
};
void *
rex_vm_thread_by_index(
    rex_vm_threadlist_t i_threadlist,
    size_t i_thread_index
);

int
rex_vm_exec(
    rex_vm_t * io_vm,
    const char * const i_string,
    const size_t i_string_sz,
    const rex_instruction_t * const i_preamble,
    const size_t i_preamble_sz,
    const rex_instruction_t * const i_prog,
    const size_t i_prog_sz,
    rex_match_t * o_matches,
    size_t * io_matches_sz
)
{
    rex_vm_threadlist_t clist, nlist, tmp;
    size_t thread_sz;
    size_t cpi, l, ti;
    uint32_t cp;
    int advance;
    rex_opcode_t op;
    // TODO: a million null checks
    thread_sz = *io_matches_sz * sizeof(rex_match_t) + sizeof(uint32_t);
    if (thread_sz * i_prog_sz * 2 > io_vm->memory_sz) return REX_OUT_OF_MEMORY;

    REX_MEMSET(io_vm->memory, 0, io_vm->memory_sz);

    clist.buffer = io_vm->memory;
    clist.thread_count = 0;
    
    for (
        cpi = 0;
        (l = rex_parse_utf8_codepoint(i_string + cpi, i_prog_sz - cpi, &cp));
        cpi += l
        )
    {
        if (clist.thread_count == 0) break;

        for(ti = 0; ti < clist.thread_count; ti++)
        {
            switch (
                (rex_opcode_t)REX_OP_FROM_INST(
                    *(uint32_t*)rex_vm_thread_by_index(clist, ti)
                )
            )
            {
                case REX_OPCODE_HI:
                case REX_OPCODE_HIA:
                case REX_OPCODE_HNI:
                case REX_OPCODE_HNIA:
                case REX_OPCODE_HR:
                case REX_OPCODE_HRA:
                case REX_OPCODE_AWB:
                case REX_OPCODE_ANWB:
                case REX_OPCODE_AE:
                case REX_OPCODE_AS:
                case REX_OPCODE_LR:
                case REX_OPCODE_SS:
                case REX_OPCODE_B:
                case REX_OPCODE_BWP:
                case REX_OPCODE_J:
                case REX_OPCODE_M:
                default:
                    return REX_BAD_INSTRUCTION;

            }
        }

    }

    

    return 0;
}


int main(void)
{

    return 0;
}
