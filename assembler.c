#include "string.h"
#include "stdio.h"

#define ISA_ITYPE_X(X)            \
    X("HI", 0x81000000)           \
    X("HIA", 0x82000000)          \
    X("HNI", 0x83000000)          \
    X("HNIA", 0x84000000)         \
    X("HR", 0x85000000)           \
    X("HRA", 0x86000000)          \
    X("AWB", 0xA1000000)          \
    X("ANWB", 0xA2000000)         \
    X("AE", 0xA3000000)           \
    X("AS", 0xA4000000)           \
    X("LR", 0xC1000000)           \
    X("SS", 0xC2000000)           \
    X("B", 0x40000000)            \
    X("BWP", 0xC0000000)          \
    X("J", 0x00000000)            

#define ISA_MATCH 0xC2000000

uint32_t
assemble_instruction(
    const char * i_str
)
{
#define ASSEMBLE(op, byte) \
    if (sscanf(i_str, op" 0x%x\n", &immediate) > 0) return immediate | byte; \
    if (sscanf(i_str, op" 0X%x\n", &immediate) > 0) return immediate | byte; \
    if (sscanf(i_str, op" %d\n", &immediate) > 0) return immediate | byte;   \
    if (sscanf(i_str, op" %c\n", (char *)&immediate) > 0) return immediate | byte;   

    uint32_t immediate = 0;
    if (*i_str == 'M') return ISA_MATCH;
    ISA_ITYPE_X(ASSEMBLE);
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
