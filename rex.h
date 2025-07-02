#include <stdint.h>
#include <stddef.h>

/* THREAD SAFTEY */
/* It is unsafe to access any REX_VM object from multiple threads concurrently 
 * In order to practice thread safety each thread will need to own its own REX_VM object 
 * It IS SAFE to pass the same program to different REX_VM objects in different threads 
 */

/* INTERFACE */

/* Error Code */
#define REX_SUCESS          (0)
#define REX_OUT_OF_MEMORY   (1)
#define REX_SYNTAX_ERROR    (1)

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
#define REX_INSTRUCTION(op, imm) (((op) << 24) & (imm))
/* REX_VM INSTRUCTION SET */
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
#define REX_HALT_IMMEDIATE (0x81)

/* HALT_NOT_IMMEDIATE
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp0     |
 *      -----------------------
 *      OP = 0b10000011
 *      If current codepoint does not match immediate value cp0 halt the thread
*/
#define REX_HALT_NOT_IMMEDIATE (0x83)

/* LOAD_RANGE_MAX_VAL
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |        cp1     |
 *      -----------------------
 *      OP = 0b11000010
 *      Load the codepoint cp1 into the RANGE_MAX_VALUE register
 *
 * HALT_RANGE
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
 *
 * MATCH
 *       ______________________
 *      |8bit|      24 bit    |
 *      -----------------------
 *      | OP |     unused     |
 *      -----------------------
 *      OP = 0b11111111
 *      Set uniform MATCH_FOUND to true
 *      Set unform  MATCH_SIZE to current codepoint index
 *
 * SPLIT
 *       ______________________
 *      |2bit|      30 bit    |
 *      -----------------------
 *      | OP |        pc1     |
 *      -----------------------
 *      OP = 0b01
 *      Spawn a new thread with PROGRAM_COUNTER initialized to pc1
 * JUMP
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
    REX_PARSE_TOKEN_NULL = 0,
    /* Illegal first byte for utf8 sequence */
    REX_PARSE_TOKEN_CHARSET = 0x80,
    REX_PARSE_TOKEN_RPAREN,       
    REX_PARSE_TOKEN_LPAREN,
    REX_PARSE_TOKEN_ALTERNATION,
    REX_PARSE_TOKEN_CONCAT,
    REX_PARSE_TOKEN_KLEEN,
    REX_PARSE_TOKEN_KLEEN_LAZY,
    REX_PARSE_TOKEN_QUESTION,
    REX_PARSE_TOKEN_QUESTION_LAZY,
    REX_PARSE_TOKEN_PLUS,
    REX_PARSE_TOKEN_PLUS_LAZY,
};
typedef enum rex_token_e rex_token_t;

/* Defines a range of codepoints */
typedef struct rex_cp_range_s rex_cp_range_t;
struct rex_cp_range_s
{
    uint32_t cp0, cp1;
};

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
    state = i_str != NULL  && i_len > 0 ? REX_UTF8_DECODE_START : REX_UTF8_FAIL;

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
        /*FALLTHROUGH */
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
    if (hex > '0' && hex < 'A') return 0; 

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
    shift = 32 - 4;
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
        case '\r':
            cp = '\r';
            break;
        case '\v':
            cp = '\v';
            break;
        case 'u':
            l++;
            i = rex_parse_fixed_len_hex(i_str + l, i_len - l, 4, &cp);
            l = i ? l+i : 0;
            break;
        case 'U':
            l++;
            i = rex_parse_fixed_len_hex(i_str + l, i_len - l, 8, &cp);
            l = i ? l+i : 0;
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

    i = rex_parse_single_char(i_str, i_len, &cp1);
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

    /* counts as a null terminator check */
    /* counts as a len check */
    /* counts as a i_str null check */
    l = rex_parse_single_char(i_str, i_len, NULL);
    if (l) return l;

    /* counts as a null terminator check */
    /* counts as a len check */
    /* counts as a i_str null check */
    l = rex_parse_multichar_set(i_str, i_len);
    if (l) return l;

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
        *o_token = REX_PARSE_TOKEN_NULL;
        return 0;
    case '(':
        *o_token = REX_PARSE_TOKEN_LPAREN;
        return 1;
    case ')' :
        *o_token = REX_PARSE_TOKEN_RPAREN;
        return 1;
    case '*':
        *o_token = REX_PARSE_TOKEN_KLEEN + lazy;
        return 1 + lazy;
    case '+':
        *o_token = REX_PARSE_TOKEN_PLUS + lazy;
        return 1 + lazy;
    case '|':
        *o_token = REX_PARSE_TOKEN_ALTERNATION;
        return 1;
    case '?':
        *o_token = REX_PARSE_TOKEN_QUESTION + lazy;
        return 1 + lazy;
    default:
        break;
    }
    
    i = rex_parse_charset(i_str, i_len);
    if (i && o_token) *o_token = REX_PARSE_TOKEN_CHARSET;
    return i;
}
/* COMPILATION */
enum rex_compiler_state_e{
    REX_STATE_INIT,
    REX_STATE_LEX,
    REX_STATE_AST
};

typedef enum rex_compiler_state_e rex_compiler_state_t;
typedef struct rex_compiler_s rex_compiler_t;
typedef struct rex_compiler_lex_ctx_s rex_compiler_lex_ctx_t;
typedef struct rex_compiler_ast_ctx_s rex_compiler_ast_ctx_t;
typedef union rex_compiler_ctx_u rex_compiler_ctx_t;
typedef struct rex_lexeme_s rex_lexeme_t;

struct rex_compiler_lex_ctx_s
{
    rex_lexeme_t * lexemes; 
    size_t lexemes_sz; 
};

struct rex_compiler_ast_ctx_s
{
    void * ast_top;
    size_t ast_sz; 
};

union rex_compiler_ctx_u
{
    rex_compiler_lex_ctx_t lex_ctx;
    rex_compiler_ast_ctx_t ast_ctx;
};

struct rex_compiler_s
{

    void * memory;
    size_t memory_sz;
    rex_compiler_state_t state;
    rex_compiler_ctx_t ctx;
};

struct rex_lexeme_s
{
    rex_token_t token;
    const char * start;
};

int
rex_lex_regex_str(
    const char * const i_str,
    const size_t i_len,
    rex_compiler_t * const io_compiler
)
{
    size_t l, i, lexeme_i;
    rex_lexeme_t lexeme;
    rex_lexeme_t * const lexeme_str = io_compiler->memory;
    l = 0;
    for (lexeme_i = 0;;lexeme_i++)
    {
        i = rex_parse_token(
            i_str + l,
            i_len - l,
            &lexeme.token
        );
        if (!i) break;
        if ((lexeme_i+1) * sizeof(rex_lexeme_t) > io_compiler->memory_sz)
        {
            return REX_OUT_OF_MEMORY;
        }
        lexeme.start = i_str + l;
        lexeme_str[lexeme_i] = lexeme;
    }
    if (l == i_len || i_str[l] == 0)
    {
        io_compiler->state = REX_STATE_LEX;
        io_compiler->ctx.lex_ctx.lexemes = lexeme_str;
        io_compiler->ctx.lex_ctx.lexemes_sz = lexeme_i;

        return REX_SUCESS;
    }
    return REX_SYNTAX_ERROR;
}


/* Jumped the gun writing this some of it may be useful later */
#if 0 
static inline size_t
rex_compile_single_char(
    const uint32_t i_cp,
    rex_instruction_t * const o_prog,
    size_t * const o_prog_sz
)
{
    *o_prog_sz = 1;
    if (o_prog != NULL) 
    {
        *o_prog = REX_INSTRUCTION(
            REX_HALT_NOT_IMMEDIATE,
            i_cp
        );
    }
    return 1;
}

static inline size_t
rex_compile_escape_sequence(
    const char * const i_str,
    const size_t i_len,
    rex_instruction_t * const o_prog,
    size_t * const o_prog_sz
)
{
    uint32_t cp;
    size_t l;
    l = rex_parse_utf8_codepoint(i_str+1, i_len-1, &cp);
    if (!l) return 1;

    switch (cp)
    {
    case 'd':
        break;
    case 'D':
        break;
    case 'w':
        break;
    case 'W':
        break;
    case 's':
        break;
    case 'S':
        break;
    default:
        l = rex_compile_single_char(cp, o_prog, o_prog_sz);
        return l ? l + 1 : 0;
    }
    return 0;
}
/* Returns the range from charset body with the least cp0 such that:
 *      cp0 <= threshold <= cp1 OR
 *      cp0 > threshold
 */
static inline rex_cp_range_t
rex_min_range_from_charset(
    const char * const i_str,
    const size_t i_len,
    uint32_t i_threshold
)
{
    rex_cp_range_t min, current;
    size_t l, range_len;
    min.cp0 = UINT32_MAX;
    min.cp1 = UINT32_MAX;
    range_len = 0;

    for(l = 0; l < i_len; l += range_len)
    {
        range_len = rex_parse_charset_range(
            i_str,
            i_len,
            i_threshold,
            &current
        );
        if(range_len == 0) break;

        if(
            current.cp0 < min.cp0 &&
                (current.cp0 > i_threshold || (
                    current.cp0 <= i_threshold && 
                    i_threshold <= current.cp1
                )
            )
        )
        {
            min = current;
        }

    }
    return min;
}

static inline size_t
rex_compile_charset(
    const char * const i_str,
    const size_t i_len,
    rex_instruction_t * const o_prog,
    size_t * const o_prog_sz
)
{
    uint32_t cp;
    size_t l;
    /* TODO: ERROR CODES */
    if (i_str == NULL || i_len == 0 || o_prog_sz == 0) return 0;
    l = rex_parse_utf8_codepoint(i_str, i_len, &cp);
    if (l == 0) return 0;
    switch(cp)
    {
    case '[':
        break;
    case '\\':
        return rex_compile_escape_sequence(
            i_str,
            i_len,
            o_prog,
            o_prog_sz
        );
    case '\0':
        return 0;
    /* Single Character */
    default:
        return rex_compile_single_char(
            cp,
            o_prog,
            o_prog_sz
        );
    }
}
#endif
