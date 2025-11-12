#define REX_IMPLEMENTATION
#include "rex.h"


#include <stdio.h>
#include <stdlib.h>

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
 * const char *[N*2] : match markers
 *
 */

/* IDEA:
 * Instructions that do not advance the character pointer and do not jump
 * can short circuit the loop and just advance to the next instruction.
 * Then the number of threads needed are only the number of threads that jump or advance.
 *
 * ^ may even be able to short circuit jumps.
 *
 * To reel this in a little bit
 * Have Multipart instructions:
 *
 * Any HALT without advance 
 * Load range
 *
 *
 * ASSERTS??
 */

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
    size_t marker_count;
};

#define REX_MARKERS(thread) ((char **) ((uint32_t*) cthread) + 1)

void *
rex_vm_thread_by_index(
    const rex_vm_threadlist_t * const i_threadlist,
    const size_t i_thread_index
){
    uint8_t * const byte_buffer = i_threadlist->buffer;
    return byte_buffer +  i_thread_index *
        (sizeof(uint32_t) + sizeof(char*) * i_threadlist->marker_count);
}

/* NO BOUNDS CHECKING!!! */
void
rex_vm_threadlist_insert(
    rex_vm_threadlist_t *i_threadlist,
    char ** i_markers,
    uint32_t i_pc,
    size_t i_index
)
{
    size_t i;
    uint32_t *pc;
    uint8_t * thread;
    char ** o_markers;
    size_t thread_sz = 
        sizeof(uint32_t) + sizeof(char *) * i_threadlist->marker_count;
    for (i = 0; i<i_index; i++)
    {
        pc = rex_vm_thread_by_index(i_threadlist, i);
        if (*pc  == i_pc) return;
    }
    pc = rex_vm_thread_by_index(i_threadlist, i);
    thread = (uint8_t *) pc;
    memmove(
            thread + thread_sz,
            thread,
            (i_threadlist->thread_count - i) * thread_sz
           );
    *pc = i_pc;
    pc++;
    o_markers = (char **) pc;
    memcpy(
            o_markers,
            i_markers,
            sizeof(char *) * i_threadlist->marker_count
          );
    i_threadlist->thread_count++;
}

void
rex_vm_threadlist_push(
    rex_vm_threadlist_t *i_threadlist,
    char ** i_markers,
    uint32_t i_pc
)
{
    size_t i;
    uint32_t *pc;
    for (i = 0; i<i_threadlist->thread_count; i++)
    {
        pc = rex_vm_thread_by_index(i_threadlist, i);
        if (*pc  == i_pc) return;
    }
    pc = rex_vm_thread_by_index(i_threadlist, i);
    *pc = i_pc;
    memcpy(
        pc+1,
        i_markers,
        sizeof(char *) * i_threadlist->marker_count
    );
    i_threadlist->thread_count++;
}

void
rex_vm_thread_expand(
    rex_vm_threadlist_t *i_threadlist,
    size_t i_start,
    const rex_instruction_t * const i_prog
)
{
    uint32_t *pc;
    uint32_t pc_tmp;
    uint32_t inst;
    rex_opcode_t op;
    size_t i;
   
    for (i = i_start;i < i_threadlist->thread_count;){
        pc = (uint32_t*) rex_vm_thread_by_index(i_threadlist, i);
        inst = i_prog[*pc];
        op = (rex_opcode_t)REX_OP_FROM_INST(inst);
        switch (op){
        case REX_OPCODE_J:
            *pc = REX_IMM_FROM_INST(inst);
            break;
        case REX_OPCODE_B:
            pc_tmp = *pc;
            /* Inc pc for first thread*/
            pc[0]++;
            /* Insert new thread pointing to jump*/
            rex_vm_threadlist_insert(
                i_threadlist,
                (char **) (pc + 1),
                REX_IMM_FROM_INST(inst),
                i+1
            );
            break;
        case REX_OPCODE_BWP:
            pc_tmp = *pc;
            /* set first thread pc to jump*/
            *pc = REX_IMM_FROM_INST(inst);
            /* Insert new thread with incremented thread pc*/
            rex_vm_threadlist_insert(
                i_threadlist,
                (char **) (pc + 1),
                pc_tmp + 1,
                i + 1
            );
            break;
        case REX_OPCODE_SS:
            break;
        default:
            i++;
            break;
        }
    }
}


int
rex_vm_exec(
    rex_vm_t * io_vm,
    const char * const i_string,
    const size_t i_string_sz,
    const rex_instruction_t * const i_prog,
    const size_t i_prog_sz,
    rex_match_t * o_matches,
    size_t * io_matches_sz,
    int * o_match_found
)
{
    rex_vm_threadlist_t clist, nlist, tmp;
    size_t thread_sz;
    size_t cpi, l, ti;
    uint32_t cp, imm, pc, inst;
    uint32_t rcp1 = 0;
    void * cthread;
    int match = 0;
    if (!io_vm || !i_string || !i_prog || !io_matches_sz) return REX_BAD_PARAM;
    thread_sz = *io_matches_sz * sizeof(rex_match_t) + sizeof(uint32_t);
    if (thread_sz * i_prog_sz * 2 > io_vm->memory_sz) return REX_OUT_OF_MEMORY;

    REX_MEMSET(io_vm->memory, 0, io_vm->memory_sz);

    /* Put a thread with pc = 0 */
    clist.buffer = io_vm->memory;
    clist.thread_count = 1;
    clist.marker_count = *io_matches_sz * 2;

    nlist.buffer = ((uint8_t*)io_vm->memory) + io_vm->memory_sz/2;
    nlist.thread_count = 0;
    nlist.marker_count = *io_matches_sz * 2;

    rex_vm_thread_expand(
        &clist, 
        0,
        i_prog
    );
    for (
        cpi = 0, l = 0;
        (l = rex_parse_utf8_codepoint(i_string + cpi, i_prog_sz - cpi, &cp));
        cpi += l
        )
    {
        if (clist.thread_count == 0) break;

        for(ti = 0; ti < clist.thread_count; ti++)
        {
            cthread = rex_vm_thread_by_index(&clist, ti);
            pc = *(uint32_t*) cthread;

        /* Some instruction sequences are handled
         * as if they were a single instruction
         */
        thread_continue:
            inst = i_prog[pc];

            imm = REX_IMM_FROM_INST(inst);
            switch ((rex_opcode_t)REX_OP_FROM_INST(inst))
            {
            case REX_OPCODE_HI:
                if (cp == imm) break;
                pc++;
                goto thread_continue;
            case REX_OPCODE_HIA:
                if (cp == imm) break;
                rex_vm_threadlist_push(
                    &nlist, 
                    REX_MARKERS(cthread),
                    pc + 1); 
                rex_vm_thread_expand(
                    &nlist, 
                    nlist.thread_count - 1,
                    i_prog
                );
                break;
            case REX_OPCODE_HNI:
                if (cp != imm) break;
                pc++;
                goto thread_continue;
            case REX_OPCODE_HNIA:
                if (cp != imm) break;
                rex_vm_threadlist_push(
                    &nlist, 
                    REX_MARKERS(cthread),
                    pc + 1); 
                rex_vm_thread_expand(
                    &nlist, 
                    nlist.thread_count - 1,
                    i_prog
                );
                break;
            case REX_OPCODE_HR:
                if (cp >= imm && cp <= rcp1) break;
                pc++;
                goto thread_continue;
            case REX_OPCODE_HRA:
                if (cp >= imm && cp <= rcp1) break;
                rex_vm_threadlist_push(
                    &nlist, 
                    REX_MARKERS(cthread),
                    pc + 1); 
                rex_vm_thread_expand(
                    &nlist, 
                    nlist.thread_count - 1,
                    i_prog
                );
                break;
            case REX_OPCODE_AWB:
            case REX_OPCODE_ANWB:
            case REX_OPCODE_AE:
            case REX_OPCODE_AS:
                //TODO: Assert
                break;
            case REX_OPCODE_LR:
                rcp1 = imm;
                pc++;
                goto thread_continue;
            case REX_OPCODE_SS:
                //TODO:
                break;
            case REX_OPCODE_B:
            case REX_OPCODE_BWP:
            case REX_OPCODE_J:
                /* UNREACHABLE */
            default:
                return REX_BAD_INSTRUCTION;

            case REX_OPCODE_M:
                //TODO: Save Marker 1
                match = 1;
                goto match;
            }
        }
    match:
        tmp = clist;
        clist = nlist;
        nlist = tmp;
        nlist.thread_count = 0;
        if (cp == 0) break;
    }
    if (o_match_found) *o_match_found = match;

    return 0;
}



/*TESTS*/

/* Compiled code for \w+
 * Alphanumeric Sequence
 */
const uint32_t Alphanumeric[12] ={
    REX_INSTRUCTION(REX_OPCODE_LR, '0'-1), 
    REX_INSTRUCTION(REX_OPCODE_HR, 0), 
    REX_INSTRUCTION(REX_OPCODE_LR, 'A'-1), 
    REX_INSTRUCTION(REX_OPCODE_HR, '9'+1), 
    REX_INSTRUCTION(REX_OPCODE_LR, '_'-1), 
    REX_INSTRUCTION(REX_OPCODE_HR, 'Z'+1), 
    REX_INSTRUCTION(REX_OPCODE_LR, 'a'-1), 
    REX_INSTRUCTION(REX_OPCODE_HR, '_'+1), 
    REX_INSTRUCTION(REX_OPCODE_LR, REX_MAX_UNICODE_VAL), 
    REX_INSTRUCTION(REX_OPCODE_HRA, 'z'+1), 
    REX_INSTRUCTION(REX_OPCODE_BWP, 0),
    REX_INSTRUCTION(REX_OPCODE_M, 0)
};

#define ALPHANUMERIC_CHARS 63
const char * Alphanumeric_pass[ALPHANUMERIC_CHARS] = {
"A",
"a",
"B",
"b",
"C",
"c",
"D",
"d",
"E",
"e",
"F",
"f",
"G",
"g",
"H",
"h",
"I",
"i",
"J",
"j",
"K",
"k",
"L",
"l",
"M",
"m",
"N",
"n",
"O",
"o",
"P",
"p",
"Q",
"q",
"R",
"r",
"S",
"s",
"T",
"t",
"U",
"u",
"V",
"v",
"W",
"w",
"X",
"x",
"Y",
"y",
"Z",
"z",
"0",
"1",
"2",
"3",
"4",
"5",
"6",
"7",
"8",
"9",
"_"
};

const char * Alphanumeric_fail[] = {
"",
"?",
"/",
",",
".",
"`",
"~",
"!",
"@",
"#",
"$",
"%",
"^",
"&",
"*",
"(",
")",
"-",
"=",
"+",
"?123",
"@abc"
};


#define TEST_STR_SZ 512
int test_alphanumeric_single(void)
{
    size_t i,  matches;
    matches = 0;
    int match, err;
    int ret = 0;
    uint8_t buffer[128];
    rex_vm_t  vm;
    
    vm.memory = buffer;
    vm.memory_sz = 128;
    for (i = 0; i < 63; i++)
    {
        err = rex_vm_exec(
            &vm,
            Alphanumeric_pass[i],
            1,
            Alphanumeric,
            12,
            NULL,
            &matches,
            &match
        );
        ret = !match || err  ? 1 : ret;
        if (ret) break;
    }
    printf(
        "\\w+ MATCHES EVERY ALPHANUMERIC SINGLE: %s",  
        match && !err ? "PASS" : "FAIL"
    );
    if (err)
    {
        printf(" WITH ERROR: %d\n",err); 
    }else{
        putchar('\n');
    }

    return  ret;
}
int
test_alphanumeric_random_sequence(void)
{
    size_t i, j, matches;
    matches = 0;
    int match, err;
    int ret = 0;
    int ri;
    uint8_t buffer[128];
    rex_vm_t  vm;
    char test_str[TEST_STR_SZ] = {0};

    vm.memory = buffer;
    vm.memory_sz = 128;

    for ( i = 0; i < 1024; i++){
        for ( j = 0; j < TEST_STR_SZ-1; j++)
        {
            ri = (uint32_t) rand();
            ri %= 63;
            test_str[j] = Alphanumeric_pass[ri][0];
        }
        test_str[(((uint32_t) rand()) % (TEST_STR_SZ-1))+1] = 0; 
        err = rex_vm_exec(
            &vm,
            test_str,
            TEST_STR_SZ,
            Alphanumeric,
            12,
            NULL,
            &matches,
            &match
        );
        ret = !match || err  ? 1 : ret;
        if (ret) break;
        
    }
    printf(
        "\\w+ MATCHES RANDOM ALPHANUMERIC STRINGS: %s",  
        match && !err ? "PASS" : "FAIL"
    );
    if (err)
    {
        printf(" WITH ERROR: %d\n",err); 
    }else{
        putchar('\n');
    }
    return ret;
}
int test_nonalphanumeric(void)
{
    size_t i,  matches;
    matches = 0;
    int match, err;
    int ret = 0;
    uint8_t buffer[128];
    rex_vm_t  vm;
    
    vm.memory = buffer;
    vm.memory_sz = 128;
    for (i = 0; i < 22; i++)
    {
        err = rex_vm_exec(
            &vm,
            Alphanumeric_fail[i],
            SIZE_MAX,
            Alphanumeric,
            12,
            NULL,
            &matches,
            &match
        );
        ret = match || err  ? 1 : ret;
        if (ret) break;
    }
    printf(
        "\\w+ DOES NOT MATCH NONALPHANUMERIC SEQUENCE: %s",  
        !match && !err ? "PASS" : "FAIL"
    );
    if (err)
    {
        printf(" WITH ERROR: %d\n",err); 
    }else{
        putchar('\n');
    }

    return  ret;
}

/* Test utf8 support on Misc symbols codepage
 * U+2600 to U+26FF
 * compiled regex for [\u2600-\u26FF]+
 */
rex_instruction_t unicode_misc_symbols[] ={
    REX_INSTRUCTION(REX_OPCODE_LR, 0x2600-1), 
    REX_INSTRUCTION(REX_OPCODE_HR, 0), 
    REX_INSTRUCTION(REX_OPCODE_LR, REX_MAX_UNICODE_VAL), 
    REX_INSTRUCTION(REX_OPCODE_HRA, 0x26FF+1), 
    REX_INSTRUCTION(REX_OPCODE_BWP, 0),
    REX_INSTRUCTION(REX_OPCODE_M, 0)
};

#define MISC_SYMBOL_CHARS 0x100
const char * Misc_symbol_pass[MISC_SYMBOL_CHARS] = {
"☀",
"☁",
"☂",
"☃",
"☄",
"★",
"☆",
"☇",
"☈",
"☉",
"☊",
"☋",
"☌",
"☍",
"☎",
"☏",
"☐",
"☑",
"☒",
"☓",
"☔",
"☕",
"☖",
"☗",
"☘",
"☙",
"☚",
"☛",
"☜",
"☝",
"☞",
"☟",
"☠",
"☡",
"☢",
"☣",
"☤",
"☥",
"☦",
"☧",
"☨",
"☩",
"☪",
"☫",
"☬",
"☭",
"☮",
"☯",
"☰",
"☱",
"☲",
"☳",
"☴",
"☵",
"☶",
"☷",
"☸",
"☹",
"☺",
"☻",
"☼",
"☽",
"☾",
"☿",
"♀",
"♁",
"♂",
"♃",
"♄",
"♅",
"♆",
"♇",
"♈",
"♉",
"♊",
"♋",
"♌",
"♍",
"♎",
"♏",
"♐",
"♑",
"♒",
"♓",
"♔",
"♕",
"♖",
"♗",
"♘",
"♙",
"♚",
"♛",
"♜",
"♝",
"♞",
"♟",
"♠",
"♡",
"♢",
"♣",
"♤",
"♥",
"♦",
"♧",
"♨",
"♩",
"♪",
"♫",
"♬",
"♭",
"♮",
"♯",
"♰",
"♱",
"♲",
"♳",
"♴",
"♵",
"♶",
"♷",
"♸",
"♹",
"♺",
"♻",
"♼",
"♽",
"♾",
"♿",
"⚀",
"⚁",
"⚂",
"⚃",
"⚄",
"⚅",
"⚆",
"⚇",
"⚈",
"⚉",
"⚊",
"⚋",
"⚌",
"⚍",
"⚎",
"⚏",
"⚐",
"⚑",
"⚒",
"⚓",
"⚔",
"⚕",
"⚖",
"⚗",
"⚘",
"⚙",
"⚚",
"⚛",
"⚜",
"⚝",
"⚞",
"⚟",
"⚠",
"⚡",
"⚢",
"⚣",
"⚤",
"⚥",
"⚦",
"⚧",
"⚨",
"⚩",
"⚪",
"⚫",
"⚬",
"⚭",
"⚮",
"⚯",
"⚰",
"⚱",
"⚲",
"⚳",
"⚴",
"⚵",
"⚶",
"⚷",
"⚸",
"⚹",
"⚺",
"⚻",
"⚼",
"⚽",
"⚾",
"⚿",
"⛀",
"⛁",
"⛂",
"⛃",
"⛄",
"⛅",
"⛆",
"⛇",
"⛈",
"⛉",
"⛊",
"⛋",
"⛌",
"⛍",
"⛎",
"⛏",
"⛐",
"⛑",
"⛒",
"⛓",
"⛔",
"⛕",
"⛖",
"⛗",
"⛘",
"⛙",
"⛚",
"⛛",
"⛜",
"⛝",
"⛞",
"⛟",
"⛠",
"⛡",
"⛢",
"⛣",
"⛤",
"⛥",
"⛦",
"⛧",
"⛨",
"⛩",
"⛪",
"⛫",
"⛬",
"⛭",
"⛮",
"⛯",
"⛰",
"⛱",
"⛲",
"⛳",
"⛴",
"⛵",
"⛶",
"⛷",
"⛸",
"⛹",
"⛺",
"⛻",
"⛼",
"⛽",
"⛾",
"⛿",
};

int test_misc_symbol_single(void)
{
    size_t i,  matches;
    matches = 0;
    int match, err;
    int ret = 0;
    uint8_t buffer[128];
    rex_vm_t  vm;
    
    vm.memory = buffer;
    vm.memory_sz = 128;
    for (i = 0; i < 0x100; i++)
    {
        err = rex_vm_exec(
            &vm,
            Misc_symbol_pass[i],
            SIZE_MAX,
            unicode_misc_symbols,
            6,
            NULL,
            &matches,
            &match
        );
        ret = !match || err  ? 1 : ret;
        if (ret) break;
    }
    printf(
        "[u+2600-u+26FF]+ MATCHES EVERY MISC SYMBOL CODEPAGE SINGLE: %s",  
        match && !err ? "PASS" : "FAIL"
    );
    if (err)
    {
        printf(" WITH ERROR: %d\n",err); 
    }else{
        putchar('\n');
    }

    return  ret;
}
int
test_misc_symbol_random_sequence(void)
{
    size_t i, j, matches;
    matches = 0;
    int match, err;
    int ret = 0;
    int ri;
    uint8_t buffer[128];
    rex_vm_t  vm;
    char test_str[TEST_STR_SZ] = {0};

    vm.memory = buffer;
    vm.memory_sz = 128;

    for ( i = 0; i < 1024; i++){
        for ( j = 0; j < TEST_STR_SZ-4; j+=3)
        {
            ri = (uint32_t) rand();
            ri %= 0x100;
            test_str[j] = Misc_symbol_pass[ri][0];
            test_str[j+1] = Misc_symbol_pass[ri][1];
            test_str[j+2] = Misc_symbol_pass[ri][2];
        }

        err = rex_vm_exec(
            &vm,
            test_str,
            TEST_STR_SZ,
            unicode_misc_symbols,
            6,
            NULL,
            &matches,
            &match
        );
        ret = !match || err  ? 1 : ret;
        if (ret) break;
        
    }
    printf(
        "[u+2600-u+26FF]+ MATCHES RANDOM MISC SYMBOL STRINGS: %s",  
        match && !err ? "PASS" : "FAIL"
    );
    if (err)
    {
        printf(" WITH ERROR: %d\n",err); 
    }else{
        putchar('\n');
    }
    return ret;
}
int test_not_misc_symbol(void)
{
    size_t i,  matches;
    matches = 0;
    int match, err;
    int ret = 0;
    uint8_t buffer[128];
    rex_vm_t  vm;
    
    vm.memory = buffer;
    vm.memory_sz = 128;
    for (i = 0; i < 22; i++)
    {
        err = rex_vm_exec(
            &vm,
            Alphanumeric_fail[i],
            SIZE_MAX,
            Alphanumeric,
            12,
            NULL,
            &matches,
            &match
        );
        ret = match || err  ? 1 : ret;
        if (ret) break;
    }
    printf(
        "[u+2600-u+26FF]+ DOES NOT MATCH SEQUENCE OUTSIDE PAGE: %s",  
        !match && !err ? "PASS" : "FAIL"
    );
    if (err)
    {
        printf(" WITH ERROR: %d\n",err); 
    }else{
        putchar('\n');
    }

    return  ret;
}


int main(void)
{
    int ret = 0;
    ret |= test_alphanumeric_single();
    ret |= test_alphanumeric_random_sequence();
    ret |= test_nonalphanumeric();
    ret |= test_misc_symbol_single();
    ret |= test_misc_symbol_random_sequence();
    ret |= test_not_misc_symbol();
    if (ret) goto exit;

exit:
    return ret;
}
