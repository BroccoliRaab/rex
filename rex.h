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

/* THREAD SAFTEY */
/* It is unsafe to access any REX_VM object from multiple threads concurrently 
 * In order to practice thread safety each thread will need to own its own REX_VM object 
 * It IS SAFE to pass the same program to different REX_VM objects in different threads 
 */

/* INTERFACE */

#define REX_MAX_UNICODE_VAL (0x00FFFFFF)

/* Error Code */
#define REX_SUCESS          (0)
#define REX_OUT_OF_MEMORY   (1)
#define REX_SYNTAX_ERROR    (2)
#define REX_BAD_STATE       (3)

/* TYPES */
typedef uint32_t rex_instruction_t;

/* REX_VM ARCHITECTURE */
/* 
 * REX_VM is Virtual Machine that spawns multiple threads that are executed sequentially
 * Each thread has its own set of registers
 * Sequential thread execution allows prioritizing one code path over another
 * This priority determines match disambiguation rules and allows lazy matching
 * MATCH_SIZE and MATCH_FOUND are set and all threads halt when a match is found
 * The max thread count spawned for any program is equal to the amount of instructions in the program
 */

/* UNIFORMS */
/* MATCH_FOUND 
*      True if REX_VM has found a matching pattern in text
 *
 * MATCH_SIZE
 *      Holds the number of codepoints searched for pattern
 */

/*REGISTERS*/
/*
 * PROGRAM_COUNTER
 *      Stores the current index of vm instruction
 *      Initialized to (~0) for a halted thread
 * RANGE_MAX_VALUE
 *      Stores the max value of a code point range
 */

/* TODO: ISA overhaul from most to least important
 * Advance Character pointer
 * Submatching
 * Add boundary assertions  \b \B \$ \^
 * x{n,m} reptition ( Not sure if this will change isa or generate more instructions or will be implemented at all 
 * */

/* ISA
 * OLD:
 * HALT_IMMEDIATE
 * HALT_NOT_IMMEDIATE
 * LOAD_RANGE_MAX_VAL
 * HALT_RANGE
 * MATCH
 * SPLIT
 * JUMP
 *
 * NEW:
 * HI           Halt Immediate
 * HIA          Halt Immediate Advance
 * HNI          Halt Not Immediate
 * HNIA         Halt Not Immediate Advance
 * HR           Halt Range
 * HRA          Halt Range Advance
 * AWB          Assert Word Boundary
 * ANWB         Assert Not Word Boundary
 * AE           Assert End
 * AS           Assert Start
 * LR           Load Range
 * SS           Save Submatch
 * M            Match
 * B            Branch/Split
 * BWP          Branch/Split With Priority
 * J            Jump
 * */

#define REX_INSTRUCTION(op, imm) (((op) << 24) & (imm))
/* REX_VM INSTRUCTION SET */
#define REX_HALT_IMMEDIATE (0x81)
/*
 * HALT_IMMEDIATE
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      OP = 0b10000001
 *      If current codepoint matches immediate value cp0 halt the thread
 */

#define REX_HALT_NOT_IMMEDIATE (0x83)
/* HALT_NOT_IMMEDIATE
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      OP = 0b10000011
 *      If current codepoint does not match immediate value cp0 halt the thread
*/

#define REX_LOAD_RANGE_MAX_VAL (0xC2)
/* LOAD_RANGE_MAX_VAL
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp1     |
 *      -----------------------
 *      OP = 0b11000010
 *      Load the codepoint cp1 into the RANGE_MAX_VALUE register
 */

#define REX_HALT_RANGE (82)
/* HALT_RANGE
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      OP = 0b10000010
 *      Current codepoint: cp
 *      Codepoint range bottom immediate: cp0
 *      Codepoint range top in register RANGE_MAX_VALUE: cp1
 *      If cp0 <= cp <= cp1 halt the current thread
 */

#define REX_MATCH (0xFF)
/* MATCH
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |     unused     |
 *      -----------------------
 *      OP = 0b11111111
 *      Set uniform MATCH_FOUND to true
 *      Set unform  MATCH_SIZE to current codepoint index
 */

#define REX_SPLIT (0x40)
/* SPLIT
 *       ______________________
 *      |2bit|      30 bit    |
 *      -----------------------
 *      | OP |        pc1     |
 *      -----------------------
 *      OP = 0b01
 *      Spawn a new thread with PROGRAM_COUNTER initialized to pc1
 */
    
#define REX_JUMP (0x0)
/* JUMP
 *       ______________________
 *      |2bit|      30 bit    |
 *      -----------------------
 *      | OP |        pc1     |
 *      -----------------------
 *      OP = 0b00
 *      Set this threads PROGRAM_COUNTER to pc1
*/

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
    REX_TOKEN_ALTERNATION_INFIX,
    REX_TOKEN_ALTERNATION_SUFFIX,
    REX_TOKEN_QUESTION_SUFFIX,
    REX_TOKEN_KLEEN_SUFFIX,
    REX_TOKEN_PLUS_SUFFIX,
};
typedef enum rex_token_e rex_token_t;

static inline int 
rex_isreserved(const uint32_t cp)
{
    switch (cp)
    {
    case ']':
    case '\\':
    case '+':
    case '*':
    case '?':
    case '^':
    case '$':
    case '.':
    case '[':
    case '{':
    case '}':
    case '(':
    case ')':
    case '|':
    case '/':
        return 1;
    default:
        return 0;
    }
}

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
)
{
    enum
    {
        REX_UTF8_FAIL,
        REX_UTF8_DECODE_START,
        REX_UTF8_COUNT_LEADING_ONES,
        REX_UTF8_VALIDATE_FIRST_BYTE,
        REX_UTF8_DECODE_FIRST_BYTE,
        REX_UTF8_VALIDATE_TAIL_BYTE,
        REX_UTF8_DECODE_TAIL_BYTE,
        REX_UTF8_DECODE_ASCII,
        REX_UTF8_SUCCESS
    } state;
    size_t l = 0;
    uint8_t leading_ones = 0;
    uint8_t tail_bytes = 0;
    uint32_t cp = 0;
    /* i_str validation */
    state = i_str != NULL && i_len > 0 ? REX_UTF8_DECODE_START : REX_UTF8_FAIL;

    for (;;) switch(state)
    {
    case REX_UTF8_FAIL:
        return 0;
    case REX_UTF8_DECODE_START:
        state = REX_UTF8_IS_MULTIBYTE(i_str[l]) ?
            REX_UTF8_COUNT_LEADING_ONES :
            REX_UTF8_DECODE_ASCII;
        break;
    /* Leading ones count of the first byte indicates the length of the sequence */
    case REX_UTF8_COUNT_LEADING_ONES:
        leading_ones++;
        /* Shift by current ones count and test the most significant bit */
        state = ((i_str[l] << leading_ones) & REX_UTF8_BYTE_MOST_SIGNIFICANT_BIT) ?
            REX_UTF8_COUNT_LEADING_ONES :
            REX_UTF8_VALIDATE_FIRST_BYTE;
        break;
    case REX_UTF8_VALIDATE_FIRST_BYTE:
        state = leading_ones >= REX_UTF8_MULTIBYTE_MIN &&
                leading_ones <= REX_UTF8_MULTIBYTE_MAX ?
                REX_UTF8_DECODE_FIRST_BYTE :
                REX_UTF8_FAIL;
        break;
    case REX_UTF8_DECODE_FIRST_BYTE:
        cp |= (
                ((uint32_t) i_str[l]) &
                REX_UTF8_SIGNIFICANT_BITMASK(leading_ones)
            ) << (
            REX_UTF8_TAIL_BYTE_SIGNIFICANT_BITS *
            REX_UTF8_TAIL_BYTES(leading_ones)
            );
        state = l < i_len? REX_UTF8_VALIDATE_TAIL_BYTE : REX_UTF8_FAIL;
        l++;
        break;
    case REX_UTF8_VALIDATE_TAIL_BYTE:
        tail_bytes = leading_ones - 1;
        state = (i_str[l] & REX_UTF8_TAIL_BYTE_LEADING_BITMASK) ==
                REX_UTF8_BYTE_MOST_SIGNIFICANT_BIT ?
                REX_UTF8_DECODE_TAIL_BYTE :
                REX_UTF8_FAIL;
        break;
    case REX_UTF8_DECODE_TAIL_BYTE:
        cp |= ((uint32_t) i_str[l] & ~REX_UTF8_TAIL_BYTE_LEADING_BITMASK) <<
            tail_bytes * REX_UTF8_TAIL_BYTE_SIGNIFICANT_BITS;
        state = tail_bytes ? REX_UTF8_SUCCESS : REX_UTF8_FAIL;
        break;
    case REX_UTF8_DECODE_ASCII:
        cp = i_str[l];
        l = 1;
        /* FALLTHROUGH */
    case REX_UTF8_SUCCESS:
        /* Check if output is NULL */
        if (o_cp != NULL) *o_cp = cp;
        return l;
    }
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
    if (rex_isreserved(cp)) return 0;
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
/* COMPILATION */
enum rex_compiler_state_e{
    REX_STATE_INIT,
    REX_STATE_AST
};

typedef enum rex_compiler_state_e rex_compiler_state_t;
typedef struct rex_compiler_s rex_compiler_t;
typedef struct rex_compiler_lex_ctx_s rex_compiler_lex_ctx_t;
typedef struct rex_compiler_ast_ctx_s rex_compiler_ast_ctx_t;
typedef union rex_compiler_ctx_u rex_compiler_ctx_t;
typedef struct rex_range_s rex_range_t;

struct rex_compiler_ast_ctx_s
{
    uint8_t * ast_top;
    size_t ast_sz; 
};

union rex_compiler_ctx_u
{
    rex_compiler_ast_ctx_t ast_ctx;
};

struct rex_compiler_s
{

    void * memory;
    size_t memory_sz;
    rex_compiler_state_t state;
    rex_compiler_ctx_t ctx;
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

/* Inserts 2 instructions */
#define REX_HALT_RANGE_INSTRUCTIONS(r0, r1)      \
    REX_INSTRUCTION(REX_LOAD_RANGE_MAX_VAL, r1), \
    REX_INSTRUCTION(REX_HALT_RANGE, r0)

static size_t const rex_w_inst_set_sz = 10;
static const rex_instruction_t rex_w_inst_set[10] =
{
    REX_HALT_RANGE_INSTRUCTIONS(0, '0'-1),
    REX_HALT_RANGE_INSTRUCTIONS('9'+1, 'A'-1),
    REX_HALT_RANGE_INSTRUCTIONS('Z'+1, '_'-1),
    REX_HALT_RANGE_INSTRUCTIONS('_'+1, 'a'-1),
    REX_HALT_RANGE_INSTRUCTIONS('z'+1, REX_MAX_UNICODE_VAL),
};

static size_t const rex_W_inst_set_sz = 7;
static const rex_instruction_t rex_W_inst_set[7] =
{
    REX_HALT_RANGE_INSTRUCTIONS('0', '9'),
    REX_HALT_RANGE_INSTRUCTIONS('A', 'Z'),
    REX_HALT_RANGE_INSTRUCTIONS('a', 'z'),
    REX_INSTRUCTION(REX_HALT_IMMEDIATE, '_'),
};

static size_t const rex_s_inst_set_sz = 5;
static const rex_instruction_t rex_s_inst_set[5] =
{
    /* 0 to '\t'-1 */
    REX_INSTRUCTION(REX_LOAD_RANGE_MAX_VAL, '\t'-1),
    REX_INSTRUCTION(REX_HALT_RANGE, 0),
    /* '\r'+1 to ' '-1 */
    REX_INSTRUCTION(REX_LOAD_RANGE_MAX_VAL, ' '-1),
    REX_INSTRUCTION(REX_HALT_RANGE, '\r'+1),
    /* Anything but '_' */
    REX_INSTRUCTION(REX_HALT_NOT_IMMEDIATE, ' ')
};

static size_t const rex_S_inst_set_sz = 3;
static const rex_instruction_t rex_S_inst_set[3] =
{
    /*  '\t' to '\r' */
    REX_HALT_RANGE_INSTRUCTIONS('\t', '\r'),
    /* '_' */
    REX_INSTRUCTION(REX_HALT_IMMEDIATE, ' ')
};

static size_t const rex_d_inst_set_sz = 4;
static const rex_instruction_t rex_d_inst_set[4] =
{
    /* 0 to '0'-1 */
    REX_INSTRUCTION(REX_LOAD_RANGE_MAX_VAL, '0'-1),
    REX_INSTRUCTION(REX_HALT_RANGE, 0),

    /* '9'+1 to MAX_VAL */
    REX_INSTRUCTION(REX_LOAD_RANGE_MAX_VAL, REX_MAX_UNICODE_VAL),
    REX_INSTRUCTION(REX_HALT_RANGE, '9'+1),
};

static size_t const rex_D_inst_set_sz = 2;
static const rex_instruction_t rex_D_inst_set[2] =
{
    /* '0' to '9' */
    REX_INSTRUCTION(REX_LOAD_RANGE_MAX_VAL, '9'),
    REX_INSTRUCTION(REX_HALT_RANGE, '0'),

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
                o_prog[sz] = REX_INSTRUCTION(REX_HALT_IMMEDIATE, r0);
            sz++;
        }else
        {
            if (o_prog)
            {
                o_prog[sz] = REX_INSTRUCTION(REX_LOAD_RANGE_MAX_VAL, r1);
                o_prog[sz+1] = REX_INSTRUCTION(REX_HALT_RANGE, r0);
            }
            sz += 2;
        }
    }
    return sz;
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
    switch (*cp)
    {
    case '[':
        return rex_compile_mcset(i_cs, o_prog);
    case '\\':
        cp++;
        switch(*cp)
        {
        case 'w':
            if (o_prog)
            {
                REX_MEMCPY(
                    o_prog,
                    rex_w_inst_set,
                    rex_w_inst_set_sz * sizeof(rex_instruction_t)
                );
            }
            return rex_w_inst_set_sz;
        case 'W':
            if (o_prog)
            {
                REX_MEMCPY(
                    o_prog,
                    rex_W_inst_set,
                    rex_W_inst_set_sz * sizeof(rex_instruction_t)
                );
            }
            return rex_W_inst_set_sz;
        case 's':
            if (o_prog)
            {
                REX_MEMCPY(
                    o_prog,
                    rex_s_inst_set,
                    rex_s_inst_set_sz * sizeof(rex_instruction_t)
                );
            }
            return rex_s_inst_set_sz;
        case 'S':
            if (o_prog)
            {
                REX_MEMCPY(
                    o_prog,
                    rex_S_inst_set,
                    rex_S_inst_set_sz * sizeof(rex_instruction_t)
                );
            }
            return rex_S_inst_set_sz;
        case 'd':
            if (o_prog)
            {
                REX_MEMCPY(
                    o_prog,
                    rex_d_inst_set,
                    rex_d_inst_set_sz * sizeof(rex_instruction_t)
                );
            }
            return rex_d_inst_set_sz;
        case 'D':
            if (o_prog)
            {
                REX_MEMCPY(
                    o_prog,
                    rex_D_inst_set,
                    rex_D_inst_set_sz * sizeof(rex_instruction_t)
                );
            }
            return rex_D_inst_set_sz;
        default:
            break;
        }
    /* Don't need to check for reserved character because the set is valid */
    /*FALLTHROUGH */
    default:
        if(*o_prog)
            *o_prog = REX_INSTRUCTION(REX_HALT_NOT_IMMEDIATE, *cp);
        return 1;
    }
}
/* Places AST into the bottom of memory as a stack
 * Tokens are each one uint8_t
 * Following a charset token is the charset string
 */

static inline int
rex_build_ast(
    const char * const i_str,
    const size_t i_len,
    rex_compiler_t * const io_compiler
)
{
    int r;
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
        if (!i) return 0;

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
            if (op_stack + op_stack_sz + 1 >= ast_top) return 1;
            op_stack[op_stack_sz++] = tok;
            break;
        case REX_TOKEN_RPAREN:
            for (;tok != REX_TOKEN_LPAREN;)
            {
                if(op_stack_sz == 0) return 2;
                if (op_stack + op_stack_sz >= ast_top) return 1;
                /* POP AND EMIT TOP OF OPSTACK*/
                *--ast_top = op_stack[--op_stack_sz];
            }
            break;
        default:
            /* WHILE STACK_TOP PRECEDENCE > TOK */
            if (op_stack_sz) while(op_stack[op_stack_sz-1] > tok)
            {
                if (op_stack + op_stack_sz >= ast_top) return 1;
                /* POP AND EMIT TOP OF OPSTACK*/
                *--ast_top = op_stack[--op_stack_sz];
            }
            /* PUSH TOK TO OPSTACK */
            if (op_stack + op_stack_sz + 1 >= ast_top) return 1;
            op_stack[op_stack_sz++] = tok;
            
            break;
        } 
        /* Handle Implicit Concat Using Lookahead */
        rex_parse_token(i_str+l+i, i_len - l - i, &tok_tmp);
        switch(tok)
        {
        case REX_TOKEN_PLUS:
        case REX_TOKEN_KLEEN:
        case REX_TOKEN_QUESTION:
        case REX_TOKEN_RPAREN:
        case REX_TOKEN_CHARSET:
            switch (tok_tmp)
            {
            case REX_TOKEN_CHARSET:
            case REX_TOKEN_LPAREN:
                /* Prevent tree mangling due to implicit concat */
                tok = REX_TOKEN_CONCAT;
                goto shunting_start;
                if (r) return r;
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
    io_compiler->ctx.ast_ctx.ast_top = ast_top;
    return 0;
}

static inline size_t
rex_count_ast(
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
    uint8_t * const ast_top = io_compiler->ctx.ast_ctx.ast_top;
    const size_t ast_sz = io_compiler->ctx.ast_ctx.ast_sz;
    r = rex_count_ast(
        ast_top,
        ast_sz
    );
    if(!r) return 1;
    l = rex_count_ast(
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
rex_ast_compile(
    rex_compiler_t *io_compiler,
    uint32_t *o_prog,
    size_t i_prog_sz
)
{
    size_t  prog_i = 0;
    size_t ast_sz = io_compiler->ctx.ast_ctx.ast_sz;
    uint8_t *ast_top = io_compiler->ctx.ast_ctx.ast_top;
    while(ast_sz) switch((rex_token_t) *ast_top)
    {
    case REX_TOKEN_CONCAT:
        ast_top++;
        rex_ast_rot(io_compiler);
        break;
    case REX_TOKEN_ALTERNATION:
        ast_top++;
        rex_ast_rot(io_compiler);
        o_prog[prog_i] = REX_INSTRUCTION(REX_SPLIT, prog_i+1);

    }
    
}
