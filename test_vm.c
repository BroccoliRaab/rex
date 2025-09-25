#define REX_IMPLEMENTATION
#include "rex.h"
typedef struct rex_submatch_s rex_submatch_t;

struct rex_submatch_s
{
    char * const match;
    size_t match_sz;
};

struct rex_vm_thread_s
{
    uint32_t pc;
};


int
match(
    const char * const i_string,
    const rex_instruction_t * const i_prog,
    rex_submatch_t * o_submatches,
    size_t * io_submatches_sz
)
{
    
}


int main(void)
{

    return 0;
}
