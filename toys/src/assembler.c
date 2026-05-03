#include "string.h"
#include "stdio.h"

#include "rex.h"

uint32_t
assemble_instruction(
    const char * i_str
)
{
#define ASSEMBLE(op, byte) \
    if (sscanf(i_str, #op" 0x%x\n", &immediate) > 0) return immediate | byte; \
    if (sscanf(i_str, #op" 0X%x\n", &immediate) > 0) return immediate | byte; \
    if (sscanf(i_str, #op" %d\n", &immediate) > 0) return immediate | byte;   \
    if (sscanf(i_str, #op" %c\n", (char *)&immediate) > 0) return immediate | byte;   

    uint32_t immediate = 0;
    if (*i_str == 'M') return 0;
    REX_ISA_X(ASSEMBLE);
    return 0;
#undef ASSEMBLE
}

int main(void)
{
    uint32_t bytecode;
    char str[128];
    for (;;) {
        if (!fgets(str, 128, stdin)) return 0;
        bytecode =assemble_instruction(str);
        if (!bytecode) return 1;
        fwrite(&bytecode, sizeof(uint32_t), 1, stdout);
    }
}
