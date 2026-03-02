#define REX_IMPLEMENTATION
#include "rex.h"


#include <stdio.h>
#include <stdlib.h>

/* TODO:
 * WORD BOUNDARY TESTS
 * Out of place instructions throw illegal instruction
 * Clear match extractions
 */

/*TESTS*/

/* Compiled code for \w+
 * Alphanumeric Sequence
 * Clear submatches on failure?
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
            0,
            Alphanumeric,
            12,
            NULL,
            matches,
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
int test_alphanumeric_single_match_extraction(void)
{
    size_t i,  matches;
    matches = 1;
    int match, err;
    int ret = 0;
    uint8_t buffer[1024];
    rex_vm_t  vm;
    rex_match_t extract;
    
    vm.memory = buffer;
    vm.memory_sz = 1024;
    for (i = 0; i < 63; i++)
    {
        err = rex_vm_exec(
            &vm,
            Alphanumeric_pass[i],
            1,
            0,
            Alphanumeric,
            12,
            &extract,
            matches,
            &match
        );
        ret |= !match || err;
        ret |= extract.match != Alphanumeric_pass[i];
        ret |= extract.match_sz != 1;
        if (ret) break;
    }
    printf(
        "\\w+ WITH MATCH EXTRACTION: %s",  
        !ret ? "PASS" : "FAIL"
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
test_alphanumeric_random_sequence_offset(void)
{
    size_t i, j, matches;
    int match, err;
    int ret = 0;
    int ri;
    uint8_t buffer[512];
    rex_vm_t  vm;
    char test_str[TEST_STR_SZ] = {0};
    rex_match_t extract;  
    matches = 1;

    vm.memory = buffer;
    vm.memory_sz = 512;

    for ( i = 0; i < 1024; i++){
        for ( j = 0; j < TEST_STR_SZ-1; j++)
        {
            ri = (uint32_t) rand() + 1;
            ri %= 63;
            test_str[j] = Alphanumeric_pass[ri][0];
        }
        test_str[(((uint32_t) rand()) % (TEST_STR_SZ-1))+1] = 0; 
        err = rex_vm_exec(
            &vm,
            test_str,
            TEST_STR_SZ,
            1,
            Alphanumeric,
            12,
            &extract,
            matches,
            &match
        );
        ret = !match || err  ? 1 : ret;
        ret |= !(extract.match == test_str+1 && extract.match_sz == ri - 1); 
        if (ret) break;
    }
    printf(
        "\\w+ MATCHES RANDOM ALPHANUMERIC STRINGS WITH OFFSET: %s",  
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
            0,
            Alphanumeric,
            12,
            NULL,
            matches,
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
            0,
            Alphanumeric,
            12,
            NULL,
            matches,
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
            0,
            unicode_misc_symbols,
            6,
            NULL,
            matches,
            &match
        );
        ret = (!match || err) ? 1 : ret;
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
            0,
            unicode_misc_symbols,
            6,
            NULL,
            matches,
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
            0,
            Alphanumeric,
            12,
            NULL,
            matches,
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

int test_insufficient_size_errors(void)
{
    size_t  matches;
    matches = 0;
    int match, err;
    uint8_t buffer[128];
    rex_vm_t  vm;
    
    vm.memory = buffer;
    vm.memory_sz = 1;
    err = rex_vm_exec(
        &vm,
        Misc_symbol_pass[1],
        SIZE_MAX,
        0,
        unicode_misc_symbols,
        6,
        NULL,
        matches,
        &match
    );
    printf(
        "INSUFFICIENT BUFFER THROWS REX_OUT_OF_MEMORY : %s",  
        err == REX_OUT_OF_MEMORY ? "PASS" : "FAIL"
    );
    putchar('\n');

    return  err != REX_OUT_OF_MEMORY;
}

int test_illegal_instruction(void)
{
    size_t  matches;
    matches = 0;
    int match, err;
    uint8_t buffer[128];
    rex_vm_t  vm;
    rex_instruction_t prog[] = {
        0xB3FFFFFF,
        0xA5FFFFFF
    };
    
    vm.memory = buffer;
    vm.memory_sz = 128;
    err = rex_vm_exec(
        &vm,
        "abc",
        SIZE_MAX,
        0,
        prog,
        2,
        NULL,
        matches,
        &match
    );
    printf(
        "ILLEGAL INSTRUCTION THROWS REX_BAD_INSTRUCTION : %s",  
        err == REX_BAD_INSTRUCTION ? "PASS" : "FAIL"
    );
    putchar('\n');

    return  err != REX_BAD_INSTRUCTION;
}

int test_bad_params(void)
{
    size_t  matches;
    matches = 0;
    int match, err;
    int ret = 0;
    uint8_t buffer[128];
    rex_vm_t  vm;
    rex_instruction_t prog[] = {
        0xB3FFFFFF,
        0xA5FFFFFF
    };
    
    vm.memory = buffer;
    vm.memory_sz = 128;

    /* NULL VM */
    err = rex_vm_exec(
        NULL,
        "abc",
        SIZE_MAX,
        0,
        prog,
        2,
        NULL,
        matches,
        &match
    );
    ret = err != REX_BAD_PARAM ? 1 : ret;

    /* NULL Search String */
    err = rex_vm_exec(
        &vm,
        NULL,
        SIZE_MAX,
        0,
        prog,
        2,
        NULL,
        matches,
        &match
    );
    ret = err != REX_BAD_PARAM ? 1 : ret;

    /* NULL Program */
    err = rex_vm_exec(
        &vm,
        "abc",
        SIZE_MAX,
        0,
        NULL,
        2,
        NULL,
        matches,
        &match
    );
    ret = err != REX_BAD_PARAM ? 1 : ret;

    printf(
        "NULL PARAMS THROWS REX_BAD_PARAM : %s",  
        !ret ? "PASS" : "FAIL"
    );
    putchar('\n');

    return ret;
}

const uint32_t Alphanumeric_With_Submatches[26] ={
    REX_INSTRUCTION(REX_OPCODE_SS, 2), 
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
    REX_INSTRUCTION(REX_OPCODE_BWP, 1),
    REX_INSTRUCTION(REX_OPCODE_SS, 3), 
    REX_INSTRUCTION(REX_OPCODE_SS, 4), 
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
    REX_INSTRUCTION(REX_OPCODE_SS, 5), 
    REX_INSTRUCTION(REX_OPCODE_M, 0)
};

int
test_alphanumeric_random_sequence_with_submatches(void)
{
    size_t i, j, matches;
    matches = 3;
    int match, err;
    int ret = 0;
    int ri;
    uint8_t buffer[4096];
    rex_vm_t  vm;
    char test_str[TEST_STR_SZ] = {0};
    rex_match_t extract[3];

    vm.memory = buffer;
    vm.memory_sz = 4096;

    for ( i = 0; i < 1024; i++){
        for ( j = 0; j < TEST_STR_SZ-1; j++)
        {
            ri = (uint32_t) rand();
            ri %= 63;
            test_str[j] = Alphanumeric_pass[ri][0];
        }
        err = rex_vm_exec(
            &vm,
            test_str,
            TEST_STR_SZ,
            0,
            Alphanumeric_With_Submatches,
            26,
            extract,
            matches,
            &match
        );
        ret |= !match || err;
        ret |= extract[0].match != test_str;
        ret |= extract[0].match_sz != TEST_STR_SZ-1;
        ret |= extract[1].match != test_str;
        ret |= extract[1].match_sz != TEST_STR_SZ-2;
        ret |= extract[2].match != test_str+TEST_STR_SZ-2;
        ret |= extract[2].match_sz != 1;
        if (ret) break;
    }
    printf(
        "(\\w+)(\\w) "
        "MATCHES RANDOM ALPHANUMERIC STRINGS WITH SUBMATCH EXTRACTION: %s",
        !ret ? "PASS" : "FAIL"
    );
    if (err)
    {
        printf(" WITH ERROR: %d\n",err); 
    }else{
        putchar('\n');
    }
    return ret;
}


/* $A^ */
const uint32_t start_end_assertion[4] ={
    REX_INSTRUCTION(REX_OPCODE_AS, 0), 
    REX_INSTRUCTION(REX_OPCODE_HNIA, 'A'), 
    REX_INSTRUCTION(REX_OPCODE_AE, 0), 
    REX_INSTRUCTION(REX_OPCODE_M, 0) 
};

/* A$A */
const uint32_t start_assertion_impossible[4] ={
    REX_INSTRUCTION(REX_OPCODE_HNIA, 'A'), 
    REX_INSTRUCTION(REX_OPCODE_AS, 0), 
    REX_INSTRUCTION(REX_OPCODE_HNIA, 'A'), 
    REX_INSTRUCTION(REX_OPCODE_M, 0) 
};


int
test_start_end_assertions(void)
{
    size_t matches;
    matches = 0;
    int match, err;
    int ret = 0;
    uint8_t buffer[1024];
    rex_vm_t  vm;


    vm.memory = buffer;
    vm.memory_sz = 1023;
    /* Should Match */
    err = rex_vm_exec(
            &vm,
            "A",
            TEST_STR_SZ,
            0,
            start_end_assertion,
            4,
            NULL,
            matches,
            &match
    );
    ret |= !match || err;

    /* Should Match */
    err = rex_vm_exec(
            &vm,
            "AB",
            1,
            0,
            start_end_assertion,
            4,
            NULL,
            matches,
            &match
    );
    ret |= !match || err;

    /* Should Not Match */
    err = rex_vm_exec(
            &vm,
            "AB",
            TEST_STR_SZ,
            0,
            start_end_assertion,
            4,
            NULL,
            matches,
            &match
    );
    ret |= match || err;

    /* Should Not Match */
    err = rex_vm_exec(
            &vm,
            "BA",
            TEST_STR_SZ,
            0,
            start_end_assertion,
            4,
            NULL,
            matches,
            &match
    );
    ret |= match || err;

    /* Should Not Match */
    err = rex_vm_exec(
            &vm,
            "AA",
            TEST_STR_SZ,
            0,
            start_assertion_impossible,
            4,
            NULL,
            matches,
            &match
    );
    ret |= match || err;

    /* Should Not Match */
    err = rex_vm_exec(
            &vm,
            "A",
            TEST_STR_SZ,
            0,
            start_assertion_impossible,
            4,
            NULL,
            matches,
            &match
    );
    ret |= match || err;

    /* Should Not Match */
    err = rex_vm_exec(
            &vm,
            "A",
            1,
            0,
            start_assertion_impossible,
            4,
            NULL,
            matches,
            &match
    );
    ret |= match || err;

    printf(
        "START AND END ANCHOR ASSERTIONS: %s",
        !ret ? "PASS" : "FAIL"
    );
    if (err)
    {
        printf(" WITH ERROR: %d\n",err); 
    }else{
        putchar('\n');
    }
    return ret;
}



int main(void)
{
    int ret = 0;
    ret |= test_alphanumeric_single();
    ret |= test_alphanumeric_single_match_extraction();
    ret |= test_alphanumeric_random_sequence();
    ret |= test_alphanumeric_random_sequence_offset();
    ret |= test_nonalphanumeric();
    ret |= test_misc_symbol_single();
    ret |= test_misc_symbol_random_sequence();
    ret |= test_not_misc_symbol();
    ret |= test_insufficient_size_errors();
    ret |= test_illegal_instruction();
    ret |= test_bad_params();
    ret |= test_alphanumeric_random_sequence_with_submatches();
    ret |= test_start_end_assertions();
    if (ret) goto exit;

exit:
    return ret;
}
