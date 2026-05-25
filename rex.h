/*
 *               -== _____           
 *            $$$$$$ """""""          
 *        $$$$           ```"""""      
 *      ;&$            Q     " =====    _____    ______  __   __       _     _
 *      ;$                         ==  |  __ \  |  ____| \ \ / /      | |  | |
 *     ;{$                         =   | |__) | | |__     \ V /       | |__| |
 *     ;                 -;''VVVV'==   |  _  /  |  __|     > <        |  __  |
 *    ;;               -{              | | \ \  | |____   / . \   _   | |  | |
 *   !(                 -;-~~^^^^^^^   |_|  \_\ |______| /_/ \_\ |#|  |_|  |_|
 *  &&                        #######]    
 *&&              {$$===           &] 
 *&              {  """==*==%]____&&  
 *             {{                     
 *             "                      
 *             ""-                    
 *         /+     %%                  
 *          $$     %"*"* "            
 *           +.__         *^^         
 *            **''~~~~~~'*`           
 *             /
 *
 * Copyright (c) 2025 Robert Herlihy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE. 
 *
 */



#include <stdint.h>
#include <stddef.h>

#ifndef REX_MEMCPY
#include <string.h>
#define REX_MEMCPY memcpy
#endif

#ifndef REX_MEMMOVE
#include <string.h>
#define REX_MEMMOVE memmove
#endif

#ifndef REX_MEMSET
#include <string.h>
#define REX_MEMSET memset
#endif

/* THREAD SAFTEY */
/* It is unsafe to access any REX_VM object from multiple threads concurrently 
 * In order to practice thread safety each thread will need to own its own REX_VM object 
 * It IS SAFE to pass the same program to different REX_VM objects in different threads 
 */

/* INTERFACE */

typedef struct rex_s rex_t;

struct rex_s
{
    void * memory;
    size_t memory_sz;
};

/* rex_t
 *
 * This contains the buffer used during compilation and execution.
 */



#define REX_MAX_UNICODE_VAL (0x00FFFFFF)

/* Error Code */
#define REX_SUCESS                  (0)
#define REX_OUT_OF_MEMORY           (1)
#define REX_SYNTAX_ERROR            (2)
#define REX_BAD_INSTRUCTION         (3)
#define REX_ENCODING_ERROR          (4)
#define REX_BAD_PARAM               (5)

/* TYPES */
typedef uint32_t rex_instruction_t;

/* TODO: 
 * x{n,m} reptition ( Not sure if this will change isa or generate more instructions or will be implemented at all 
 * Am I using the word load wrong?
 * Change assert to something like Halt Not End, etc
 * */

/* REX_VM ARCHITECTURE */
/* 
 * REX_VM is Virtual Machine that spawns multiple threads that are executed sequentially
 * Each thread has its own set of registers
 * Sequential thread execution allows prioritizing one code path over another
 * This priority determines match disambiguation rules and allows lazy matching
 * VM halts when all 
 * The max thread count spawned for any program is equal to the amount of instructions in the program
 */

/*REGISTERS*/
/*
 * PROGRAM_COUNTER PC
 *      Stores the current index of vm instruction
 *      Initialized to (~0) for a halted thread
 * RANGE_MAX_VALUE R
 *      Stores the max value of a code point range
 *
 */

/* INSTRUCTIONS */
/*
 * HIA          Halt Immediate Advance
 * HI           Halt Immediate
 * HNIA         Halt Not Immediate Advance
 * HNI          Halt Not Immediate
 * HRA          Halt Range Advance
 * HR           Halt Range
 * AWB          Assert Word Boundary
 * ANWB         Assert Not Word Boundary
 * AE           Assert End
 * AS           Assert Start
 * LR           Load Range
 * SS           Save Submatch
 * M            Match
 * BWP          Branch/Split With Priority
 * B            Branch/Split
 * J            Jump
 *
 * MICROCODE
 * MSB -> LSB
 * 00 00 0000
 * |  |   |
 * |  | Type Specific Flags
 * |  |
 * |  | Match:
 * |  | unused
 * |  |
 * |  | Halt:
 * |  | 0 0 00
 * |  | | | |
 * |  | | | | Codepoint:
 * |  | | | | 0 0
 * |  | | | | | |
 * |  | | | | | Range Flag
 * |  | | | | | 
 * |  | | | | Invert Flag
 * |  | | | | 
 * |  | | | | Assert:
 * |  | | | | 00
 * |  | | | | |
 * |  | | | | 00 End of string     
 * |  | | | | 01 Start of string   
 * |  | | | | 10 Word Boundary     
 * |  | | | | 11 Not Word Boundary 
 * |  | | | 
 * |  | | Assert Flag
 * |  | | 0 codepoint
 * |  | | 1 assert
 * |  | |
 * |  | Advance Flag
 * |  |
 * |  |
 * |  | Load Flags
 * |  | 0000 rcp1 register
 * |  | 0001 submatch memory
 * |  |
 * |  |
 * | Immediate Type Flags
 * | 00 Match Type
 * | 01 Halt Type
 * | 10 Load Type
 * |
 * Jump Flags
 * 00 Not a Jump Type Instruction
 * 01 Jump without branching
 * 10 Continue then branch
 * 11 Branch then continue
 */
#define REX_MICROCODE_RANGE                     ( 1 )
#define REX_MICROCODE_INVERT                    ( 1 << 1 )
#define REX_MICROCODE_ASSERT                    ( 1 << 2 )
#define REX_MICROCODE_ADVANCE                   ( 1 << 3 )
#define REX_MICROCODE_SUBMATCH                  ( 1 )
#define REX_MICROCODE_ASSERT_START              ( 1 )
#define REX_MICROCODE_ASSERT_WORD_BOUNDARY      ( 2 )
#define REX_MICROCODE_ASSERT_NOT_WORD_BOUNDARY  ( 3 )
#define REX_MICROCODE_HALT                      ( 1 << 4 )
#define REX_MICROCODE_LOAD                      ( 2 << 4 )
#define REX_MICROCODE_JUMP                      ( 1 << 6 )
#define REX_MICROCODE_BRANCH_LOW_PRIORITY       ( 2 << 6 )
#define REX_MICROCODE_BRANCH_HIGH_PRIORITY      ( 3 << 6 )

/*
 * Halt_Immediate_Advance HIA
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      If current codepoint matches immediate value cp0 halt the thread
 *      Advances the character pointer
 */
#define REX_OPCODE_HIA ( REX_MICROCODE_HALT | REX_MICROCODE_ADVANCE )

 /*
 * Halt_Immediate HI
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      If current codepoint matches immediate value cp0 halt the thread
 */
#define REX_OPCODE_HI ( REX_MICROCODE_HALT )

 /*
 * Halt_Not_Immediate_Advance HNIA
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      If current codepoint does not match immediate value cp0 halt the thread
 *      Advances the character pointer
 */
#define REX_OPCODE_HNIA ( \
        REX_MICROCODE_HALT | REX_MICROCODE_INVERT | REX_MICROCODE_ADVANCE )

 /*
 * Halt_Not_Immediate HNI
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      If current codepoint does not match immediate value cp0 halt the thread
 */
#define REX_OPCODE_HNI ( REX_MICROCODE_HALT | REX_MICROCODE_INVERT )

 /*
 * Halt_Range_Advance HRA
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      If the range [cp0, R] contains the current codepoint halt the thread
 *      Advances the character pointer
 */
#define REX_OPCODE_HRA ( \
        REX_MICROCODE_HALT | REX_MICROCODE_RANGE | REX_MICROCODE_ADVANCE )

 /*
 * Halt_Range HR
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      If the range [cp0, R] contains the current codepoint halt the thread
 */
#define REX_OPCODE_HR ( REX_MICROCODE_HALT | REX_MICROCODE_RANGE )

 /*
 * Assert_Word_Boundary AWB
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |      unused    |
 *      -----------------------
 *      If the current codepoint is not a word boundary halt
 */
#define REX_OPCODE_AWB ( REX_MICROCODE_HALT | \
        REX_MICROCODE_ASSERT | REX_MICROCODE_ASSERT_WORD_BOUNDARY )

 /*
 * Assert_Not_Word_Boundary ANWB
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |      unused    |
 *      -----------------------
 *      If the current codepoint is a word boundary halt
 */
#define REX_OPCODE_ANWB ( REX_MICROCODE_HALT | \
        REX_MICROCODE_ASSERT | REX_MICROCODE_ASSERT_NOT_WORD_BOUNDARY )

 /*
 *
 * Assert_End AE
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |      unused    |
 *      -----------------------
 *      If the current codepoint is not the final codepoint in the string halt
 */
#define REX_OPCODE_AE ( REX_MICROCODE_HALT | REX_MICROCODE_ASSERT )

/*
 * Assert_Start AS
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |      unused    |
 *      -----------------------
 *      If the current codepoint is not the first codepoint in the string halt
 */
#define REX_OPCODE_AS ( REX_MICROCODE_HALT | \
        REX_MICROCODE_ASSERT | REX_MICROCODE_ASSERT_START )

/*
 *
 * Load_Range_Max_Val LR
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      Move immedate cp0 to register R
 */
#define REX_OPCODE_LR ( REX_MICROCODE_LOAD )

/*
 * 
 * Save_Submatch SS
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |  marker index  |
 *      -----------------------
 *      Store character pointer at immediate marker index
 */
#define REX_OPCODE_SS ( REX_MICROCODE_LOAD | REX_MICROCODE_SUBMATCH )

/*
 * Branch_With_Priority BWP
 *       ______________________
 *      |2bit|      32 bit    |
 *      -----------------------
 *      | OP |     address    |
 *      -----------------------
 *      Branch this thread and execute before continuing
 */
#define REX_OPCODE_BWP ( REX_MICROCODE_BRANCH_HIGH_PRIORITY )

/*
 * Branch B
 *       ______________________
 *      |2bit|      32 bit    |
 *      -----------------------
 *      | OP |     address    |
 *      -----------------------
 *      Branch this thread and continue before execution
 */
#define REX_OPCODE_B ( REX_MICROCODE_BRANCH_LOW_PRIORITY )

/*
 * Jump J
 *       ______________________
 *      |2bit|      32 bit    |
 *      -----------------------
 *      | OP |     address    |
 *      -----------------------
 *      Load address into pc
 */
#define REX_OPCODE_J ( REX_MICROCODE_JUMP )

/*
 * Match M
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      Set the matched register MF
 */
#define REX_OPCODE_M ( 0 )

#define REX_ISA_X(X)            \
    X(HIA, REX_OPCODE_HIA)      \
    X(HI, REX_OPCODE_HI)        \
    X(HNIA, REX_OPCODE_HNIA)    \
    X(HNI, REX_OPCODE_HNI)      \
    X(HRA, REX_OPCODE_HRA)      \
    X(HR, REX_OPCODE_HR)        \
    X(AWB, REX_OPCODE_AWB)      \
    X(ANWB, REX_OPCODE_ANWB)    \
    X(AE, REX_OPCODE_AE)        \
    X(AS, REX_OPCODE_AS)        \
    X(LR, REX_OPCODE_LR)        \
    X(SS, REX_OPCODE_SS)        \
    X(BWP, REX_OPCODE_BWP)      \
    X(B, REX_OPCODE_B)          \
    X(J, REX_OPCODE_J)          \
    X(M, REX_OPCODE_M)


#define REX_JUMP_IMM_MASK 0x3FFFFFFF
#define REX_NORMAL_IMM_MASK 0x00FFFFFF

#define REX_INST_IS_JUMP_TYPE(inst) ((inst) >> 30 != 0)

#define REX_OP_FROM_INST(inst) (((inst) >> 24 ) &   \
        (REX_INST_IS_JUMP_TYPE(inst) ? 0xC0 : 0xFF))

#define REX_IMM_FROM_INST(inst) ((inst) &   \
        (REX_INST_IS_JUMP_TYPE(inst) ? REX_JUMP_IMM_MASK : REX_NORMAL_IMM_MASK))

#define REX_INSTRUCTION(op, imm) (((op) << 24) | (imm))

#define REX_PC_MAX REX_JUMP_IMM_MASK

/* PARSERS */

/* All parsers stop when the length of the string is exceded
 * Strings are not expected to be NULL terminated unless otherwise stated
 *
 * All parsers return the amount of characters parsed
 *      A return value 0 indicates a parse failure
 *
 * All parsers accept as the first two arguments:
 *      i_str : The string to parse
 *      i_len : the length of the string to parse
 *
 * Any further arguments are outputs of the parser
 * All outputs may be NULL
 */

/* TODO: 
 * printable? 
    Not sure if useful. 
    Would also complicate the LUT logic by disallowing shortcut to NULL.

 */

/* Generated by ascii_class.py */
static const uint8_t rex_ascii_class[128] =
{
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x0,
    0x0, 0x1, 0x1, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
    0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
    0x2, 0x1, 0x1, 0x1, 0x0, 0x2, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
    0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
    0x2, 0x2, 0x2, 0x1, 0x1, 0x1, 0x0, 0x0,
};

#define REX_ASCII_CLASS_RESERVED_MASK   (0x1)
#define REX_ASCII_CLASS_WORD_MASK       (0x2)
#define REX_ASCII_CLASS_DIGIT_MASK      (0x4)
#define REX_ASCII_CLASS_NORM(x) (((uint32_t) x) > 127 ? 0 : (uint32_t) x)

#define REX_ISRESERVED(x) \
    (rex_ascii_class[REX_ASCII_CLASS_NORM(x)] & REX_ASCII_CLASS_RESERVED_MASK)

#define REX_ISWORD(x) \
    (!!(rex_ascii_class[REX_ASCII_CLASS_NORM(x)] & REX_ASCII_CLASS_WORD_MASK))

#define REX_ISDIGIT(x) \
    (!!(rex_ascii_class[REX_ASCII_CLASS_NORM(x)] & REX_ASCII_CLASS_DIGIT_MASK))

enum rex_token_e
{
    /* Illegal first byte for utf8 sequence */
    REX_TOKEN_CHARSET = 0x80,
    REX_TOKEN_RPAREN, /* Token is removed at AST construction time */
    REX_TOKEN_LPAREN,
    REX_TOKEN_ALTERNATION,
    REX_TOKEN_CONCAT,
    REX_TOKEN_KLEEN,
    REX_TOKEN_KLEEN_LAZY,
    REX_TOKEN_QUESTION,
    REX_TOKEN_QUESTION_LAZY,
    REX_TOKEN_PLUS,
    REX_TOKEN_PLUS_LAZY,
    /* Tokens inserted at compile time */
    REX_TOKEN_ALTERNATION_STAGE_1,
    REX_TOKEN_ALTERNATION_STAGE_2,
    REX_TOKEN_QUESTION_STAGE_1,
    REX_TOKEN_KLEEN_STAGE_1,
    REX_TOKEN_PLUS_STAGE_1,
    REX_TOKEN_PLUS_LAZY_STAGE_1,
};
typedef enum rex_token_e rex_token_t;

#define REX_TOKEN_X(X) \
    X(REX_TOKEN_CHARSET) \
    X(REX_TOKEN_RPAREN) \
    X(REX_TOKEN_LPAREN) \
    X(REX_TOKEN_ALTERNATION) \
    X(REX_TOKEN_CONCAT) \
    X(REX_TOKEN_KLEEN) \
    X(REX_TOKEN_KLEEN_LAZY) \
    X(REX_TOKEN_QUESTION) \
    X(REX_TOKEN_QUESTION_LAZY) \
    X(REX_TOKEN_PLUS) \
    X(REX_TOKEN_PLUS_LAZY) \
    X(REX_TOKEN_ALTERNATION_INFIX) \
    X(REX_TOKEN_ALTERNATION_SUFFIX) \
    X(REX_TOKEN_QUESTION_SUFFIX) \
    X(REX_TOKEN_KLEEN_SUFFIX) \
    X(REX_TOKEN_PLUS_SUFFIX)

#define REX_UTF8_IS_MULTIBYTE(c)                        ((c) & 0x80)
#define REX_UTF8_MULTIBYTE_MIN                          (2)
#define REX_UTF8_MULTIBYTE_MAX                          (4)
#define REX_UTF8_TAIL_BYTE_SIGNIFICANT_BITS             (6)
#define REX_UTF8_TAIL_BYTES(byte_count)                 ((byte_count) - 1) 
#define REX_UTF8_SIGNIFICANT_BITMASK(leading_ones)      (0xFF >> (leading_ones))
#define REX_UTF8_TAIL_BYTE_LEADING_BITMASK              (0xC0)
#define REX_UTF8_BYTE_MOST_SIGNIFICANT_BIT              (0x80)
/* UTF8 Parsing
 * Returns production length
 * Arguments:
 *      i_str: string to parse
 *      i_len: length of string
 *      o_cp:  where to store the parsed code point (NULLABLE)
 * Notes:
 * If reads a null terminator will return 1 and o_cp will be set to 0
 */

static inline size_t
rex_parse_utf8_codepoint(
    const char * const i_str,
    size_t i_len,
    uint32_t * const o_cp
){
    uint8_t b;
    const unsigned char * const u_str = (const unsigned char * const) i_str;
    uint8_t l, mask, shift, i;
    uint32_t cp;
    /* LUT for leading ones count up to 5 bits */
    static const uint8_t lo5[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4, 5
    };
    if (!(i_len && u_str)) return 0;
    b = *u_str;
    l = lo5[b>>3]; 
    if (l > i_len) return 0;
    cp = 0;
    mask = 0xFF >> (l + 1);
    shift = 6 * (l - 1);
    cp =(b & mask) << shift; 
    i = 1;
    switch(l)
    {
    case 0:
        if (o_cp) *o_cp = b;
        return 1;
    case 4:
        l *= u_str[i] >> 6 == 2;
        cp |= (u_str[i++] & 0x3f) << 12;
        /* FALLTHROUGH */
    case 3:
        l *= u_str[i] >> 6 == 2;
        cp |= (u_str[i++] & 0x3f) << 6;
        /* FALLTHROUGH */
    case 2:
        l *= u_str[i] >> 6 == 2;
        cp |= u_str[i++] & 0x3f;
        break;
    default:
        return 0;
    }
    if (o_cp) *o_cp = cp;
    return l;
}

static inline size_t
rex_parse_hex_digit(
    const char * const i_str,
    const size_t i_len,
    uint32_t * o_hex
)
{
    uint32_t hex;
    if (i_len == 0 || i_str == NULL) return 0;
    hex = i_str[0];

    /* counts as a null terminator check */
    if (hex < '0') return 0; 
    if (hex > '9' && hex < 'A') return 0; 

    /* shift a-f to A-F */
    if (hex >= 'a' && hex <= 'f') hex -= ('a' - 'A');

    if (hex > 'F') return 0;

    /* shift A-F to come right after the ascii digits */
    if (hex >= 'A' && hex <= 'F') hex -= ('A' -('9' + 1 ));

    /* Shift the result to 0-15 range */
    if (o_hex) *o_hex = hex - '0';
    return 1;
}
static inline size_t
rex_parse_fixed_len_hex(
    const char * const i_str,
    const size_t i_len,
    const uint32_t i_hex_len,
    uint32_t * o_hex
)
{
    uint32_t hex;
    uint32_t out;
    size_t l, i, j;
    uint32_t shift;
    l = 0;
    out = 0;
    shift = i_hex_len * 4 - 4;
    for (j = 0; j < i_hex_len; j++)
    {
        /* counts as a null terminator check */
        /* counts as a len check */
        /* counts as a i_str null check */
        i = rex_parse_hex_digit(i_str + l, i_len - l, &hex);
        if (i == 0) return 0;
        
        out |= hex << shift;
        shift -= 4;
        l += i;
    }
    if (o_hex) *o_hex = out;
    return l;
}
static inline size_t
rex_parse_single_char(
    const char * const i_str,
    const size_t i_len,
    uint32_t * const o_cp
)
{
    size_t l, i;
    uint32_t cp;
    l = 0;
    i = 0;
    /* couns as len check and null i_str check */
    i = rex_parse_utf8_codepoint(i_str, i_len, &cp);
    if (i == 0) return 0;
    if (cp == 0) return 0;
    l += i;
    switch (cp)
    {
    case 0:
        return 0;
    case '\\':
        i = rex_parse_utf8_codepoint(i_str+l, i_len-l, &cp);
        if (i == 0) return 0;
        l+=i;
        switch(cp)
        {
        case 0:
            return 0;
        case 'w':
        case 'W':
        case 's':
        case 'S':
        case 'd':
        case 'D':
            return 0;
        case 'a':
            cp = '\a';
            break;
        case 'f':
            cp = '\f';
            break;
        case 't':
            cp = '\t';
            break;
        case 'n':
            cp = '\n';
            break;
        case 'r':
            cp = '\r';
            break;
        case 'v':
            cp = '\v';
            break;
        case 'u':
            i = rex_parse_fixed_len_hex(i_str + l, i_len - l, 4, &cp);
            if (i == 0 || cp > REX_MAX_UNICODE_VAL) return 0;
            l+=i;
            break;
        case 'U':
            i = rex_parse_fixed_len_hex(i_str + l, i_len - l, 8, &cp);
            if (i == 0 || cp > REX_MAX_UNICODE_VAL) return 0;
            l+=i;
            break;
        default:
        break;
        }
        if (o_cp) *o_cp = cp;
        return l;
    default:
        if (o_cp) *o_cp = cp;
        return l;
    }
}

static inline size_t
rex_parse_multichar_escape(
    const char * const i_str,
    const size_t i_len
)
{
    if (i_len < 2 || i_str == NULL) return 0;
    /* counts as null terminator check */
    if (i_str[0] != '\\') return 0;
    switch(i_str[1])
    {
    case 'w':
        break;
    case 'W':
        break;
    case 's':
        break;
    case 'S':
        break;
    case 'd':
        break;
    case 'D':
        break;
    default:
        return 0;
    }
    return 2;
}

static inline size_t
rex_parse_multichar_range(
    const char * const i_str,
    const size_t i_len,
    uint32_t * const o_cp0,
    uint32_t * const o_cp1
)
{
    uint32_t cp0, cp1;
    size_t l, i;
    /* counts as a null terminator check */
    /* counts as a len check */
    /* counts as a i_str null check */
    l = rex_parse_single_char(i_str, i_len, &cp0);
    if (l == 0) return 0;

    i = rex_parse_utf8_codepoint(i_str + l, i_len - l, &cp1);
    if (i == 0) return 0;
    if (cp1 != '-') return 0;
    l += i;

    i = rex_parse_utf8_codepoint(i_str + l, i_len - l, &cp1);
    if (i == 0) return 0;
    l += i;

    if (o_cp0) *o_cp0 = cp0;
    if (o_cp1) *o_cp1 = cp1;
    return l;
}

static inline size_t
rex_parse_multichar_set(
    const char * const i_str,
    const size_t i_len
)
{
    uint32_t cp0, cp1;
    size_t l = 0;
    size_t i = 0;

    if (i_str == NULL || i_len == 0) return 0;
    switch (i_str[0])
    {
    case 0:
        return 0;
    case '\\':
        return rex_parse_multichar_escape(i_str, i_len);
    default:
        break;
    }

    /* the smallest charset is '[]' so 2 chars */
    if (i_len < 2) return 0;
    if (i_str[1] == 0) return 0;

    if (i_str[0] != '[') return 0;

    l++;
    /* Can test because we validated string is at least 2 chars */
    l = i_str[l] == '^'? l + 1: l;
    for (;;)
    {
        /* This does the length check */
        i = rex_parse_utf8_codepoint(i_str + l, i_len - l, &cp0);
        if (i == 0) return 0;
        if (cp0 == 0) return 0;
        if (cp0 == ']') return l + i;
        
        i = rex_parse_multichar_range(i_str + l, i_len - l, &cp0, &cp1);
        if (i) {
            if (cp0 > cp1) return 0;
            l += i;
            continue;
        }

        i = rex_parse_single_char(i_str + l, i_len -l, NULL);
        if (i)
        {
            l += i;
            continue;
        }

        i = rex_parse_multichar_escape(i_str + l, i_len - l);
        if (i)
        {
            l += i;
            continue;
        }

        return 0;
    }
}

static inline size_t
rex_parse_charset(
    const char * const i_str,
    const size_t i_len
)
{
    size_t l = 0;
    uint32_t cp;

    /* counts as a null terminator check */
    /* counts as a len check */
    /* counts as a i_str null check */
    l = rex_parse_multichar_set(i_str, i_len);
    if (l) return l;

    /* counts as a null terminator check */
    /* counts as a len check */
    /* counts as a i_str null check */
    l = rex_parse_single_char(i_str, i_len, &cp);
    if (REX_ISRESERVED(cp)) return 0;
    if (l && cp != 0) return l;

    return 0;
}

static inline size_t
rex_parse_token(
    const char * const i_str,
    const size_t i_len,
    rex_token_t * const o_token
)
{
    size_t i;
    int lazy = 0;
    if (i_str == NULL || i_len == 0) return 0;
    if (i_len > 1 && i_str[0] != 0)
    {
        lazy = i_str[1] == '?' ? 1 : 0;
    }
    switch(i_str[0])
    {
    case 0:
        return 0;
    case '(':
        *o_token = REX_TOKEN_LPAREN;
        return 1;
    case ')' :
        *o_token = REX_TOKEN_RPAREN;
        return 1;
    case '*':
        *o_token = REX_TOKEN_KLEEN + lazy;
        return 1 + lazy;
    case '+':
        *o_token = REX_TOKEN_PLUS + lazy;
        return 1 + lazy;
    case '|':
        *o_token = REX_TOKEN_ALTERNATION;
        return 1;
    case '?':
        *o_token = REX_TOKEN_QUESTION + lazy;
        return 1 + lazy;
    default:
        break;
    }
    
    i = rex_parse_charset(i_str, i_len);
    if (i && o_token) *o_token = REX_TOKEN_CHARSET;
    return i;
}
/* UTIL */

typedef struct rex_stack_s rex_stack_t;

/* 
 *  When stack is empty top == floor
 *  Stack grows towards ceil
 *  ceil may be greater than or less than floor;
 */
struct rex_stack_s {
    void * top;
    void * floor;
    void * ceil;
};

void * 
rex_stack_push(
    rex_stack_t * const io_stack,
    const void * const i_val,
    const size_t i_val_sz
)
{
    uint8_t * ceil = io_stack->ceil;
    uint8_t * top = io_stack->top;
    
    if (ceil > top)
    {
        if (top + i_val_sz > ceil) return NULL;
        REX_MEMCPY(top, i_val, i_val_sz);
        top += i_val_sz;
    }else{
        if (top - i_val_sz < ceil) return NULL;
        top -= i_val_sz;
        REX_MEMCPY(top, i_val, i_val_sz);
    }
    io_stack->top = top;
    return top;
}

void * 
rex_stack_pop(
    rex_stack_t * const io_stack,
    size_t i_val_sz
)
{
    uint8_t * floor = io_stack->floor;
    uint8_t * top = io_stack->top;
    if (top > floor)
    {
        if (top - i_val_sz < floor) return NULL;
        top -= i_val_sz;
    }else{
        if (top + i_val_sz > floor) return NULL;
        top += i_val_sz;
    }
    io_stack->top = top;
    return top;
}

void *
rex_stack_peek(
    const rex_stack_t * const io_stack,
    const size_t i_val_sz
){
    if (io_stack->top > io_stack->floor)
        return ((uint8_t*)io_stack->top) - i_val_sz;
    if (io_stack->top == io_stack->floor)
        return 0;
    return io_stack->top;
}


typedef struct rex_compiler_s rex_compiler_t;
typedef struct rex_range_s rex_range_t;

struct rex_compiler_s
{

    uint8_t * memory;
    size_t memory_sz;
    uint8_t * ast_top;
};

struct rex_range_s
{
    uint32_t r0,r1;
};

static size_t const rex_w_range_set_sz = 4;
static const rex_range_t rex_w_range_set[4] =
{
    {'0', '9'},{'A','Z'},{'_','_'},{'a', 'z'}
};

static size_t const rex_W_range_set_sz = 5;
static const rex_range_t rex_W_range_set[5] =
{
    {0,     '0'-1},
    {'9'+1, 'A'-1},
    {'Z'+1, '_'-1},
    {'_'+1, 'a'-1},
    {'z'+1, REX_MAX_UNICODE_VAL}
};

static size_t const rex_s_range_set_sz = 2;
static const rex_range_t rex_s_range_set[2] =
{
    {'\t', '\r'}, {' ', ' '}
};

static size_t const rex_S_range_set_sz = 3;
static const rex_range_t rex_S_range_set[3] =
{
    {0, '\t'-1}, {'\r'+1, ' '-1}, {' '+1, REX_MAX_UNICODE_VAL}
};

static size_t const rex_d_range_set_sz = 1;
static const rex_range_t rex_d_range_set[1] =
{
    {'0', '9'}
};

static size_t const rex_D_range_set_sz = 2;
static const rex_range_t rex_D_range_set[2] =
{
    {0, '0'-1}, {'9'+1, REX_MAX_UNICODE_VAL}
};

/* Helper functions for extracting character ranges from character sets */
#define REX_MAX(start0, start1) \
    (((start0) > (start1))? (start0) : (start1))
#define REX_MIN(start0, start1) \
    (((start0) < (start1))? (start0) : (start1))
#define REX_INTERSECTION_EXISTS(r0_start, r0_end, r1_start, r1_end) \
    (REX_MAX(r0_start, r1_start) <= REX_MIN(r0_end, r1_end))
#define REX_MIN_INTERSECTION_START(min0, min1, cur0, cur1, super0, super1) \
    (REX_INTERSECTION_EXISTS(cur0, cur1, super0, super1) ?                 \
        REX_MIN(min0, cur0) : min0)
#define REX_MIN_INTERSECTION_END(min0, min1, cur0, cur1, super0, super1) \
    (REX_INTERSECTION_EXISTS(cur0, cur1, super0, super1) ?               \
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
        if (cp0 > REX_MIN(min_cp1+1, REX_MAX_UNICODE_VAL)) break;
        /* Merge ranges */
        min_cp1 = cp1;
    };
    *io_r0 = min_cp0;
    *io_r1 = min_cp1;
}


/* 
 * Takes a range [*io_r0, *io_r1] as input
 * Finds the least range that is a subset [*io_r0, *io_r1] and not i_body
 * If no subset is found, *io_r0 == *io_r1 > CODEPOINT_MAX_VAL
 *
 * No validation performed against i_body
 * Inputting UINT32_MAX breaks this function
 */
static inline void rex_mcset_body_simplify_inverted(
    const char * const i_body,
    uint32_t * const io_r0,
    uint32_t * const io_r1
)
{
    uint32_t cur0;
    uint32_t cur1;

    cur0 = *io_r0;
    cur1 = *io_r1;
    /* TODO: This looks like an infinite loop. Fix or comment */
    for (;cur0 < REX_MAX_UNICODE_VAL;)
    {
        rex_mcset_body_simplify(
            i_body,
            &cur0,
            &cur1
        );
        /* TODO:
         * if this isn't true it loops forever?
         * if its always true then the if block is pointless */
        if (REX_INTERSECTION_EXISTS(cur0, cur1, *io_r0, *io_r1))
        {
            if (*io_r0 < cur0)
            {
                *io_r1 = cur0-1;
                return;
            }
            if (*io_r0 > cur0 && *io_r1 < cur1)
            {
                *io_r0 = REX_MAX_UNICODE_VAL+1;
                *io_r1 = REX_MAX_UNICODE_VAL+1;
                return;
            }
            if(*io_r1 > cur1)
            {
                *io_r0 = cur1+1;
                cur0 = *io_r0;
                cur1 = *io_r1;
                continue;
            }
        }
    }
}

static inline size_t
rex_compile_mcset(
    const char * const i_cs,
    rex_instruction_t * const o_prog
)
{
    uint32_t r0, r1;
    size_t sz = 0;
    r0 = 0;
    r1 = REX_MAX_UNICODE_VAL;
    for (
        r0 = 0, r1 = REX_MAX_UNICODE_VAL;
        ;
        r0 = r1 +1, r1 = REX_MAX_UNICODE_VAL
        )
    {
        /* The inversion looks backwards but isn't 
         * this inserts halt instructions for bad ranges
         */
        if (i_cs[2] == '^')
        {
            rex_mcset_body_simplify(
                i_cs+2,
                &r0, &r1
            );
        }else
        {
            rex_mcset_body_simplify_inverted(
                i_cs+1,
                &r0, &r1
            );
        }
        if (r0 < REX_MAX_UNICODE_VAL) break;
        if (r0 == r1)
        {
            if (o_prog) 
                o_prog[sz] = REX_INSTRUCTION(REX_OPCODE_HI, r0);
            sz++;
        }else
        {
            if (o_prog)
            {
                o_prog[sz] = REX_INSTRUCTION(REX_OPCODE_LR, r1);
                o_prog[sz+1] = REX_INSTRUCTION(REX_OPCODE_HR, r0);
            }
            sz += 2;
        }
    }
    if (sz){
        o_prog[sz - 1] |= REX_INSTRUCTION(
            REX_OPCODE_HRA,
            o_prog[sz - 1] & REX_NORMAL_IMM_MASK 
        );
    }
    return sz;
}
static inline size_t
rex_compile_character_class(
    const char i_class,
    rex_instruction_t * const o_prog
)
{
    const rex_range_t * range_set;
    const size_t * range_set_sz;
    size_t ri;

    /* We compile the inverted range set as halt instructions */
    switch (i_class)
    {
    case 'w':
        range_set = rex_W_range_set;
        range_set_sz = &rex_W_range_set_sz;
        break;
    case 'W':
        range_set = rex_w_range_set;
        range_set_sz = &rex_w_range_set_sz;
        break;
    case 'd':
        range_set = rex_D_range_set;
        range_set_sz = &rex_D_range_set_sz;
        break;
    case 'D':
        range_set = rex_d_range_set;
        range_set_sz = &rex_d_range_set_sz;
        break;
    case 's':
        range_set = rex_S_range_set;
        range_set_sz = &rex_S_range_set_sz;
        break;
    case 'S':
        range_set = rex_s_range_set;
        range_set_sz = &rex_s_range_set_sz;
        break;
    default:
        return 0;
    }
    if (o_prog){
        for(ri = 1; ri < *range_set_sz; ri++){
            o_prog[ri*2-1] = REX_INSTRUCTION(
                    REX_OPCODE_LR, range_set[ri].r0);
            o_prog[ri*2] = REX_INSTRUCTION(
                    REX_OPCODE_HR, range_set[ri].r1);
        }
        /* Turn the final HR instruction into an HRA instruction */
        ri--;
        o_prog[ri*2] |= REX_MICROCODE_ADVANCE; 
    }

    /* Each range generates 2 instructions */
    return *range_set_sz * 2;
}
/* 
 * Returns compile size
 *
 * Charset must be validated with rex_parse_charset() before invocation */
static inline size_t
rex_compile_charset(
    const char * const i_cs,
    rex_instruction_t * const o_prog
)
{
    const char * cp = i_cs;
    size_t i;
    switch (*cp)
    {
    case '[':
        return rex_compile_mcset(i_cs, o_prog);
    case '\\':
        cp++;
        i = rex_compile_character_class(*cp, o_prog);
        if (i) return i;
    /* Don't need to check for reserved character because the set is valid */
    /*FALLTHROUGH */
    default:
        if(*o_prog)
            *o_prog = REX_INSTRUCTION(REX_OPCODE_HNIA, *cp);
        return 1;
    }
}
/* Places AST into the top of memory as a stack
 * Tokens are each one uint8_t
 * Following a charset token is the charset string
 *
 * The output stack grows backward from the top of memory.
 * This avoids a backward decode of utf8.
 */


static inline int
rex_build_ast(
    const char * const i_str,
    const size_t i_len,
    rex_compiler_t * const io_compiler
)
{
    size_t i;
    size_t l, op_stack_sz;
    rex_token_t tok, tok_tmp;
    uint8_t * const op_stack = io_compiler->memory;
    uint8_t * ast_top = op_stack + io_compiler->memory_sz;

    l = 0;
    op_stack_sz = 0;

    /* Make Tree */
    for (l = 0; l < i_len && i_str[l]; l+=i)
    {
        i = rex_parse_token(i_str+l, i_len, &tok);
        if (!i) return REX_SYNTAX_ERROR;

    shunting_start:
        switch(tok)
        {
        case REX_TOKEN_CHARSET:
            /* EMIT TOK */
            if (op_stack + op_stack_sz >= ast_top - i)
                return REX_OUT_OF_MEMORY;
            REX_MEMCPY(ast_top - i, i_str + l, i);
            ast_top -= i;
            break;
        case REX_TOKEN_LPAREN:
            /* PUSH TO OPSTACK */
            if (op_stack + op_stack_sz + 1 >= ast_top)
                return REX_OUT_OF_MEMORY;
            op_stack[op_stack_sz++] = tok;
            break;
        case REX_TOKEN_RPAREN:
            for (;tok != REX_TOKEN_LPAREN;)
            {
                if(op_stack_sz == 0) return REX_SYNTAX_ERROR;
                if (op_stack + op_stack_sz >= ast_top)
                    return REX_OUT_OF_MEMORY;
                /* POP AND EMIT TOP OF OPSTACK*/
                *--ast_top = op_stack[--op_stack_sz];
            }
            break;
        default:
            /* WHILE STACK_TOP PRECEDENCE > TOK */
            if (op_stack_sz) while(op_stack[op_stack_sz-1] > tok)
            {
                if (op_stack + op_stack_sz >= ast_top) return REX_OUT_OF_MEMORY;
                /* POP AND EMIT TOP OF OPSTACK*/
                *--ast_top = op_stack[--op_stack_sz];
            }
            /* PUSH TOK TO OPSTACK */
            if (op_stack + op_stack_sz + 1 >= ast_top) return REX_OUT_OF_MEMORY;
            op_stack[op_stack_sz++] = tok;
            
            break;
        } 
        /* Handle Implicit Concat Using Lookahead */
        if (rex_parse_token(i_str+l+i, i_len - l - i, &tok_tmp)) switch(tok)
        {
        case REX_TOKEN_PLUS:
        case REX_TOKEN_PLUS_LAZY:
        case REX_TOKEN_KLEEN:
        case REX_TOKEN_KLEEN_LAZY:
        case REX_TOKEN_QUESTION:
        case REX_TOKEN_QUESTION_LAZY:
        case REX_TOKEN_RPAREN:
        case REX_TOKEN_CHARSET:
            switch (tok_tmp)
            {
            case REX_TOKEN_CHARSET:
            case REX_TOKEN_LPAREN:
                /* Prevent tree mangling due to implicit concat */
                tok = REX_TOKEN_CONCAT;
                goto shunting_start;
            default:
                break;
            }
        default:
            break;
        }
    }

    for(;op_stack_sz;)
    {
        /* No size check needed. 
         * As output stack increases Input stack decreases */
        /* POP AND EMIT TOP OF OPSTACK*/
        *--ast_top = op_stack[--op_stack_sz];
    }
    io_compiler->ast_top = ast_top;
    return REX_SUCESS;
}

static inline size_t
rex_ast_count(
    const uint8_t * const i_ast,
    const size_t i_max_sz
)
{
    size_t sz, i, j, leaves;
    sz = 0;
    leaves = 1;
    for (i = 0;i < i_max_sz;i++) switch((rex_token_t) i_ast[i])
    {
    case REX_TOKEN_CONCAT:
    case REX_TOKEN_ALTERNATION:
        leaves++;
    /* FALLTHROUGH */
    case REX_TOKEN_PLUS:
    case REX_TOKEN_PLUS_LAZY:
    case REX_TOKEN_KLEEN:
    case REX_TOKEN_KLEEN_LAZY:
    case REX_TOKEN_QUESTION:
    case REX_TOKEN_QUESTION_LAZY:
    case REX_TOKEN_LPAREN:
        sz++;
        break;
    /* AST is invalid */
    case REX_TOKEN_RPAREN:
    case REX_TOKEN_CHARSET:
        return 0;
    //TODO: SUFFIX/INFIX compile tokens
    default:
        if (!leaves) return 0;
        j = rex_parse_charset((const char *) i_ast+i, SIZE_MAX);
        if (!j) return 0;
        sz += j;
        leaves--;
        if ( leaves == 0) return sz;
    }
    /* AST is invalid */
    return 0;
}

static inline int 
rex_ast_rot(
    rex_compiler_t * const io_compiler
)
{
    size_t r, l, toswap, move_sz, swap_sz;
    uint8_t * const ast_top = io_compiler->ast_top;
    const size_t ast_sz = 
        io_compiler->memory + io_compiler->memory_sz - io_compiler->ast_top;
    r = rex_ast_count(
        ast_top,
        ast_sz
    );
    if(!r) return 1;
    l = rex_ast_count(
        ast_top + r,
        ast_sz - r
    );
    if (!l) return 1;
    swap_sz = ((uint8_t *) io_compiler->memory) - ast_top;
    for(toswap = r; toswap != 0; toswap -= move_sz)
    {
        move_sz = REX_MIN(toswap, swap_sz);
        memmove(ast_top-move_sz, ast_top, ast_sz);
        memmove(ast_top+ast_sz-move_sz ,ast_top-move_sz, move_sz);
    }
    return 0;
}
static inline int
rex_ast_insert(
    rex_compiler_t *io_compiler,
    uint8_t * i_data,
    size_t i_data_sz
)
{
    if ((io_compiler->ast_top - i_data_sz) < io_compiler->memory)
        return REX_OUT_OF_MEMORY;
    REX_MEMMOVE(
        io_compiler->ast_top - i_data_sz,
        io_compiler->ast_top,
        i_data_sz
    );
    REX_MEMCPY(io_compiler->ast_top, i_data, i_data_sz);
    io_compiler->ast_top -= i_data_sz;
    return 0;
}
#define REX_AST_PUSH_PRIMITIVE(compiler, type, prim) \
    if (compiler->ast_top - sizeof(type) < compiler->memory) \
        return REX_OUT_OF_MEMORY; \
    compiler->ast_top -= sizeof(type); \
    *((type *)(compiler->ast_top)) = prim; \

#define REX_AST_OVERFLOW_CHECK(io_compiler, size) \
    if ( \
        io_compiler->ast_top + size > \
        io_compiler->memory + io_compiler->memory_sz \
    ) return REX_SYNTAX_ERROR;


static inline int
rex_ast_compile(
    rex_compiler_t *io_compiler,
    uint32_t *o_prog,
    size_t i_prog_sz
)
{
    uint8_t * const ast_floor = io_compiler->memory + io_compiler->memory_sz;
    int r;
    uint32_t mi = 2;
    uint32_t pi, lookback;
    uint8_t branch;


    pi = 0;
    while (io_compiler->ast_top < ast_floor) 
    {
        branch = REX_OPCODE_B;
        switch((rex_token_t) *io_compiler->ast_top)
        {
        case REX_TOKEN_CHARSET:
            return REX_SYNTAX_ERROR;

        case REX_TOKEN_RPAREN:
            if (o_prog) o_prog[pi] = REX_INSTRUCTION(REX_OPCODE_SS, mi);
            mi++;
            io_compiler->ast_top++;
            break;

        case REX_TOKEN_LPAREN:
            if (o_prog) o_prog[pi] = REX_INSTRUCTION(REX_OPCODE_SS, mi);
            mi++;
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint8_t, REX_TOKEN_RPAREN);
            r = rex_ast_rot(io_compiler);
            if (r) return r;
            io_compiler->ast_top++;
            r = rex_ast_rot(io_compiler);
            if (r) return r;
            break;

        case REX_TOKEN_ALTERNATION:
            io_compiler->ast_top++;
            r = rex_ast_rot(io_compiler);
            if (r) return r;
            if (o_prog)
                o_prog[pi] = REX_INSTRUCTION(REX_OPCODE_B, 0);
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint32_t, pi);
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint32_t, 
                REX_TOKEN_ALTERNATION_STAGE_1);
            r = rex_ast_rot(io_compiler);
            if (r) return r;
            pi++;
            break;

        case REX_TOKEN_CONCAT:
            io_compiler->ast_top++;
            r = rex_ast_rot(io_compiler);
            if (r) return r;
            break;

        case REX_TOKEN_KLEEN:
            branch = REX_OPCODE_BWP;
            /*FALLTHROUGH*/
        case REX_TOKEN_KLEEN_LAZY:
                return REX_SYNTAX_ERROR;
            if (o_prog)
                o_prog[pi] = REX_INSTRUCTION(branch, pi+1);
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint32_t, pi);
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint32_t, 
                REX_TOKEN_KLEEN_STAGE_1);
            r = rex_ast_rot(io_compiler);
            if (r) return r;
            pi++;
            break;

        case REX_TOKEN_QUESTION:
            branch = REX_OPCODE_BWP;
            /*FALLTHROUGH*/
        case REX_TOKEN_QUESTION_LAZY:
            io_compiler->ast_top++;
            if (o_prog)
                o_prog[pi] = REX_INSTRUCTION(branch, 0);
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint32_t, pi);
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint32_t, 
                REX_TOKEN_QUESTION_STAGE_1);
            r = rex_ast_rot(io_compiler);
            if (r) return r;
            pi++;
            break;

        case REX_TOKEN_PLUS:
            /*FALLTHROUGH*/
        case REX_TOKEN_PLUS_LAZY:
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint32_t, pi);
            REX_AST_PUSH_PRIMITIVE(
                io_compiler,
                uint32_t, 
                REX_TOKEN_PLUS_STAGE_1 + *io_compiler->ast_top - REX_TOKEN_PLUS
            );
            io_compiler->ast_top++;
            r = rex_ast_rot(io_compiler);
            if (r) return r;
            pi++;
            break;

        case REX_TOKEN_ALTERNATION_STAGE_1:
            io_compiler->ast_top++;
            REX_AST_OVERFLOW_CHECK(io_compiler, sizeof(uint32_t));
            lookback = *(uint32_t *)io_compiler->ast_top;
            io_compiler->ast_top+= sizeof(uint32_t);
            if (pi+1 < REX_PC_MAX && o_prog)
                o_prog[lookback] |= pi+1;
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint32_t, pi);
            REX_AST_PUSH_PRIMITIVE(io_compiler, uint32_t, 
                REX_TOKEN_ALTERNATION_STAGE_2);
            if (o_prog)
                o_prog[pi] = REX_INSTRUCTION(REX_OPCODE_J, 0);
            r = rex_ast_rot(io_compiler);
            if (r) return r;
            pi++;
            break;

        case REX_TOKEN_ALTERNATION_STAGE_2:
            io_compiler->ast_top++;
            REX_AST_OVERFLOW_CHECK(io_compiler, sizeof(uint32_t));
            lookback = *(uint32_t *)io_compiler->ast_top;
            io_compiler->ast_top+= sizeof(uint32_t);
            if (o_prog)
                o_prog[lookback] |= pi+1;
            break;
            
        case REX_TOKEN_QUESTION_STAGE_1:
            io_compiler->ast_top++;
            REX_AST_OVERFLOW_CHECK(io_compiler, sizeof(uint32_t));
            lookback = *(uint32_t *)io_compiler->ast_top;
            io_compiler->ast_top+= sizeof(uint32_t);
            if (o_prog)
                o_prog[lookback] |= pi+1;
            break;
            
        case REX_TOKEN_KLEEN_STAGE_1:
            io_compiler->ast_top++;
            REX_AST_OVERFLOW_CHECK(io_compiler, sizeof(uint32_t));
            lookback = *(uint32_t *)io_compiler->ast_top;
            io_compiler->ast_top+= sizeof(uint32_t);
            if (o_prog)
            {
                o_prog[lookback] |=pi+1;
                o_prog[pi] = REX_INSTRUCTION(REX_OPCODE_J, lookback);
            }
            pi++;
            break;

        case REX_TOKEN_PLUS_STAGE_1:
            branch = REX_OPCODE_BWP;
            /*FALLTHROUGH*/
        case REX_TOKEN_PLUS_LAZY_STAGE_1:
            io_compiler->ast_top++;
            REX_AST_OVERFLOW_CHECK(io_compiler, sizeof(uint32_t));
            lookback = *(uint32_t *)io_compiler->ast_top;
            io_compiler->ast_top+= sizeof(uint32_t);
            if (o_prog)
                o_prog[pi] = REX_INSTRUCTION(branch, lookback);
            pi++;
            break;
        }
    }
    return 0; 
}


/* REX VIRTUAL MACHINE */
typedef struct rex_match_s rex_match_t;
typedef struct rex_vm_s rex_vm_t;
typedef struct rex_vm_threadlist_s rex_vm_threadlist_t;

/* Thread Memory Layout
 *
 * uint32_t : pc
 * const char *[N*2] : match markers
 *
 */

struct rex_match_s
{
    const char * match;
    size_t match_sz;
};

struct rex_vm_threadlist_s
{
    void * buffer;
    size_t thread_count;
    size_t marker_count;
};

#define REX_MARKERS(thread) ((const char **) (((uint32_t*) thread) + 1))

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
    const char ** i_markers,
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
    const char ** i_markers,
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
    const rex_instruction_t * const i_prog,
    const char * str_pos

)
{
    uint32_t *pc;
    uint32_t pc_tmp;
    uint32_t inst;
    uint8_t op;
    void * thread;
    size_t i;
    uint32_t mi;
   
    for (i = i_start;i < i_threadlist->thread_count;){
        thread = (uint32_t*) rex_vm_thread_by_index(i_threadlist, i);
        pc = thread;
        inst = i_prog[*pc];
        op = REX_OP_FROM_INST(inst);
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
                REX_MARKERS(thread),
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
                REX_MARKERS(thread),
                pc_tmp + 1,
                i + 1
            );
            break;
        /* TODO:
         * REFACTOR: Reconcile the handling on anchors with this 
         * Move SS to vm func or move anchors here 
         * or decide this is fine and leave a comment
         * Anchors are failable unlike everything here
         * also require start and stop information
         * and string length*/
        case REX_OPCODE_SS:
            mi = REX_IMM_FROM_INST(inst);
            if (mi < i_threadlist->marker_count)
                REX_MARKERS(thread)[mi] = str_pos;
            pc[0]++;
            break;
        default:
            i++;
            break;
        }
    }
}



struct rex_vm_s
{
    void * memory;
    size_t memory_sz;
    const char * string;
    size_t string_sz;
    size_t string_start;
    const rex_instruction_t * prog;
    size_t prog_sz;
    rex_match_t * matches;
    size_t matches_sz;
    rex_vm_threadlist_t clist, nlist;
    size_t thread_sz;
    size_t cpi, l, ti, mi;
    uint32_t  pc;
    uint32_t cp, rcp1;
    uint8_t prev_word;
    void * cthread;
    int match;
    int halted;
};

static int
rex_vm_exec_thread(
    rex_vm_t * io_vm
){
    
    rex_instruction_t inst;
    uint32_t imm;
            io_vm->cthread = rex_vm_thread_by_index(&io_vm->clist, io_vm->ti);
            io_vm->pc = *(uint32_t*) io_vm->cthread;

        /* Some instruction sequences are handled
         * as if they were a single instruction
         */
        /* TODO: Decide if it makes sense to refactor this into a while loop */
        thread_continue:
            inst = io_vm->prog[io_vm->pc];

            imm = REX_IMM_FROM_INST(inst);
            switch (REX_OP_FROM_INST(inst))
            {
            case REX_OPCODE_HI:
                if (io_vm->cp == imm) break;
                io_vm->pc++;
                goto thread_continue;
            case REX_OPCODE_HIA:
                if (io_vm->cp == imm) break;
                rex_vm_threadlist_push(
                    &io_vm->nlist, 
                    REX_MARKERS(io_vm->cthread),
                    io_vm->pc + 1); 
                rex_vm_thread_expand(
                    &io_vm->nlist, 
                    io_vm->nlist.thread_count - 1,
                    io_vm->prog,
                    io_vm->string + io_vm->cpi + io_vm->l
                );
                break;
            case REX_OPCODE_HNI:
                if (io_vm->cp != imm) break;
                io_vm->pc++;
                goto thread_continue;
            case REX_OPCODE_HNIA:
                if (io_vm->cp != imm) break;
                rex_vm_threadlist_push(
                    &io_vm->nlist, 
                    REX_MARKERS(io_vm->cthread),
                    io_vm->pc + 1); 
                rex_vm_thread_expand(
                    &io_vm->nlist, 
                    io_vm->nlist.thread_count - 1,
                    io_vm->prog,
                    io_vm->string + io_vm->cpi + io_vm->l
                );
                break;
            case REX_OPCODE_HR:
                if (io_vm->cp >= imm && io_vm->cp <= io_vm->rcp1) break;
                io_vm->pc++;
                goto thread_continue;
            case REX_OPCODE_HRA:
                if (io_vm->cp >= imm && io_vm->cp <= io_vm->rcp1) break;
                rex_vm_threadlist_push(
                    &io_vm->nlist, 
                    REX_MARKERS(io_vm->cthread),
                    io_vm->pc + 1); 
                rex_vm_thread_expand(
                    &io_vm->nlist, 
                    io_vm->nlist.thread_count - 1,
                    io_vm->prog,
                    io_vm->string + io_vm->cpi + io_vm->l
                );
                break;
            case REX_OPCODE_AWB:
                if (io_vm->prev_word == REX_ISWORD(io_vm->cp)) break;
                io_vm->pc = ++*(uint32_t *)io_vm->cthread;
                rex_vm_thread_expand(
                    &io_vm->clist, 
                   io_vm->ti,
                    io_vm->prog,
                    io_vm->string + io_vm->cpi + io_vm->l
                );
                goto thread_continue;
            case REX_OPCODE_ANWB:
                if (io_vm->prev_word != REX_ISWORD(io_vm->cp)) break;
                io_vm->pc = ++*(uint32_t *)io_vm->cthread;
                rex_vm_thread_expand(
                    &io_vm->clist, 
                   io_vm->ti,
                    io_vm->prog,
                    io_vm->string + io_vm->cpi + io_vm->l
                );
                goto thread_continue;
            case REX_OPCODE_AE:
                if (io_vm->string_sz - io_vm->cpi != 0)
                    if (io_vm->string[io_vm->cpi] != 0)
                        break;
                io_vm->pc = ++*(uint32_t *)io_vm->cthread;
                rex_vm_thread_expand(
                    &io_vm->clist, 
                   io_vm->ti,
                    io_vm->prog,
                    io_vm->string + io_vm->cpi + io_vm->l
                );
                goto thread_continue;
            case REX_OPCODE_AS:
                if (io_vm->cpi != 0) break;
                io_vm->pc = ++*(uint32_t *)io_vm->cthread;
                rex_vm_thread_expand(
                    &io_vm->clist, 
                   io_vm->ti,
                    io_vm->prog,
                    io_vm->string + io_vm->cpi + io_vm->l
                );
                goto thread_continue;
            case REX_OPCODE_LR:
                io_vm->rcp1 = imm;
                io_vm->pc++;
                goto thread_continue;
            case REX_OPCODE_SS:
            case REX_OPCODE_B:
            case REX_OPCODE_BWP:
            case REX_OPCODE_J:
                /* Handled by thread expand */
                /*FALLTHROUGH*/
            default:
                return REX_BAD_INSTRUCTION;

            case REX_OPCODE_M:
                io_vm->match = 1;
                if (io_vm->clist.marker_count > 1)
                    REX_MARKERS(io_vm->cthread)[1] = io_vm->string + io_vm->cpi;
                for (io_vm->mi = 0; io_vm->mi < io_vm->clist.marker_count; io_vm->mi+=2)
                    io_vm->matches[io_vm->mi/2] = 
                        (rex_match_t){
                            REX_MARKERS(io_vm->cthread)[io_vm->mi],
                            REX_MARKERS(io_vm->cthread)[io_vm->mi + 1] - REX_MARKERS(io_vm->cthread)[io_vm->mi]
                    };
                io_vm->ti = SIZE_MAX;
                return REX_SUCESS;
            } 
            io_vm->ti++;
            return REX_SUCESS;
}


static int
rex_vm_exec_step(
    rex_vm_t * io_vm
){
    rex_vm_threadlist_t tmp;
    if(io_vm->ti < io_vm->clist.thread_count)
    {
        return rex_vm_exec_thread(io_vm);
    }
    
    /* Swap clist and nlist */
    tmp = io_vm->clist;
    io_vm->clist = io_vm->nlist;
    io_vm->nlist = tmp;
    io_vm->nlist.thread_count = 0;
    io_vm->prev_word = REX_ISWORD(io_vm->cp);
    if (io_vm->cp == 0 || io_vm->l == 0)
    {
        io_vm->halted = 1;
        return REX_SUCESS;
    };

    io_vm->cpi += io_vm->l;

    io_vm->l = rex_parse_utf8_codepoint(
        io_vm->string + io_vm->cpi,
        io_vm->string_sz - io_vm->cpi,
        &io_vm->cp
    );
    if (io_vm->l == 0 && io_vm->string_sz - io_vm->cpi != 0) io_vm->halted =1;
    /* TODO:
     * If we dont want it to be halted after finishing prog
     * maybe remove? */
    if (io_vm->clist.thread_count == 0) io_vm->halted =1;

    io_vm->ti = 0;

    return REX_SUCESS;
}
static int
rex_vm_exec_init(
    rex_vm_t * o_vm,
    void * i_memory,
    size_t i_memory_sz,
    const char * const i_string,
    const size_t i_string_sz,
    const size_t i_string_start,
    const rex_instruction_t * const i_prog,
    const size_t i_prog_sz,
    rex_match_t * o_matches,
    size_t i_matches_sz
    ){
    if (!o_vm || !i_string || !i_prog ) return REX_BAD_PARAM;
    
    o_vm->thread_sz = i_matches_sz * 2 * sizeof(char *) + sizeof(uint32_t);
    
    if (o_vm->thread_sz * i_prog_sz * 2 > o_vm->memory_sz) return REX_OUT_OF_MEMORY;

    REX_MEMSET(o_vm, 0, sizeof(rex_vm_t));
    o_vm->memory = i_memory;
    o_vm->memory_sz = i_memory_sz;
    o_vm->string = i_string;
    /*TODO: Did not halt on string_sz 0 */
    o_vm->string_sz = i_string_sz;
    o_vm->string_start = i_string_start;
    o_vm->cpi = i_string_start;
    o_vm->prog = i_prog;
    o_vm->matches = o_matches;
    o_vm->matches_sz = i_matches_sz;

    /* Can look back one byte as any valid unicode byte will return false */
    o_vm->prev_word = 
        o_vm->cpi == 0 ? 0 : REX_ISWORD(o_vm->string[o_vm->cpi - 1]);


    /* Put a thread with pc = 0 */
    o_vm->clist.buffer = o_vm->memory;
    *(uint32_t*)o_vm->clist.buffer = 0;
    o_vm->clist.thread_count = 1;
    o_vm->clist.marker_count = o_vm->matches_sz * 2;

    o_vm->nlist.buffer = ((uint8_t*)o_vm->memory) + o_vm->memory_sz/2;
    o_vm->nlist.thread_count = 0;
    o_vm->nlist.marker_count = o_vm->matches_sz * 2;

    o_vm->cthread = o_vm->clist.buffer;

    if (o_vm->clist.marker_count)
        REX_MARKERS(o_vm->cthread)[0] = o_vm->string + o_vm->cpi;

    rex_vm_thread_expand(
        &o_vm->clist, 
        0,
        i_prog,
        i_string + o_vm->cpi
    );
    o_vm->l = rex_parse_utf8_codepoint(
        i_string + o_vm->cpi,
        i_string_sz - o_vm->cpi,
        &o_vm->cp
    );
    if (o_vm->l == 0 && i_string_sz - o_vm->cpi != 0) o_vm->halted =1;
    /* TODO:
     * If we dont want it to be halted after finishing prog
     * maybe remove? */
    if (o_vm->clist.thread_count == 0) o_vm->halted =1;
    return REX_SUCESS;

}

/* 
 * NOTES:
 * MARKER REGISTERS 0 and 1 are reserved for pattern match
 * If i_matches_sz is greater than actual submatch count
 * extra matches are undefined
 */

int
rex_vm_exec(
    rex_vm_t * io_vm,
    const char * const i_string,
    const size_t i_string_sz,
    const size_t i_string_start,
    const rex_instruction_t * const i_prog,
    const size_t i_prog_sz,
    rex_match_t * o_matches,
    size_t i_matches_sz,
    int * o_match_found
)
{
    int r;
    if (!io_vm || !i_string || !i_prog ) return REX_BAD_PARAM;
    r = rex_vm_exec_init(
        io_vm,
        io_vm->memory,
        io_vm->memory_sz,
        i_string,
        i_string_sz,
        i_string_start,
        i_prog,
        i_prog_sz,
        o_matches,
        i_matches_sz
    );
    if (r) return r;

    while (
     ( (r = rex_vm_exec_step(io_vm))==0 && !io_vm->halted)
    );
    if (o_match_found) *o_match_found = io_vm->match;

    return r;
}

