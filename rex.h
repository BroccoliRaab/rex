#include <stdint.h>
#include <stddef.h>
#include <string.h>
/* REX_CFG
 *
 */
#ifndef REX_BOOL_T
#define REX_BOOL_T int
#endif /* REX_BOOL_T */

#ifndef REX_SIZE_T
#define REX_SIZE_T size_t
#endif /* REX_SIZE_T */

#ifdef REX_IMPLEMENTATION

/* REX_TRY
 * If condition is true return x
 * If condition is false return 0
 */
#define REX_TRY(cond, x) ((cond) * (x))

/* REX_CHOOSE
 * If condition is true return x 
 * If condition is false return x+1
 */
#define REX_CHOOSE(cond, x) ((!(cond)) + (x))

/* REX_NCHOOSE
 * If condition is true return x+1 
 * If condition is false return x
 */
#define REX_NCHOOSE(cond, x) ((cond) + (x))


#define REX_CHAR_MIN (1)
#define REX_CHAR_MAX (0x7FFFFFFF) /*TODO */


/* REGEX PARSING */
enum rex_utf8_codepoint_fsm_state_e
{
    REX_UTF8_FSM_FAIL = 0,
    REX_UTF8_FSM_VALID_FIRST_BYTE,
    REX_UTF8_FSM_CHAR_TAIL,
    REX_UTF8_FSM_SUCCESS,
    REX_UTF8_FSM_DECODE,
    REX_UTF8_FSM_ASCII_CHAR,
    REX_UTF8_FSM_CHAR,
    REX_UTF8_FSM_CHECK_BYTE_ONE,
    REX_UTF8_FSM_VALID_TAIL_BYTE,

};
enum rex_mcset_fsm_state_e
{
    REX_MCSET_FSM_BODY = 0,
    REX_MCSET_FSM_EXIT,
    REX_MCSET_FSM_RANGE,
    REX_MCSET_FSM_RANGE_ORDERED,
    REX_MCSET_FSM_ESC,
    REX_MCSET_FSM_CHAR,
    REX_MCSET_FSM_EMIT,
    REX_MCSET_FSM_EMIT_RETURN,
};

enum rex_charset_fsm_state_e
{
    REX_CS_FSM_INST_ISNULL =0,
    REX_CS_FSM_SET_INST,
    REX_CS_FSM_EXIT ,
    REX_CS_FSM_ALPHABET_SET,
    REX_CS_FSM_ESC_SEQ,
    REX_CS_FSM_MULTICHAR_SET,
    REX_CS_FSM_SINGLECHAR_SET
};

enum rex_escape_seq_mask_e
{
    REX_ESC_SEQ_w = 1 << (0 + 24),
    REX_ESC_SEQ_W = 1 << (1 + 24),
    REX_ESC_SEQ_s = 1 << (2 + 24),
    REX_ESC_SEQ_S = 1 << (3 + 24),
    REX_ESC_SEQ_d = 1 << (4 + 24),
    REX_ESC_SEQ_D = 1 << (5 + 24)
};

enum rex_mcset_fsm_range_state_e
{
    REX_MCSET_RANGE_EXIT = 0,
    REX_MCSET_RANGE_ESC,
    REX_MCSET_RANGE_CHAR,   
    REX_MCSET_RANGE_ESC_ISVALID,
    REX_MCSET_RANGE_HYPHEN,
};

enum rex_parse_token_e
{
    REX_PARSE_TOKEN_NULL = 0,
    REX_PARSE_TOKEN_CHARSET_STR,
    REX_PARSE_TOKEN_RPAREN,       
    REX_PARSE_TOKEN_LPAREN,
    REX_PARSE_TOKEN_ALTERNATION,
    REX_PARSE_TOKEN_CONCAT,
    REX_PARSE_TOKEN_KLEEN,
    REX_PARSE_TOKEN_QUESTION,
    REX_PARSE_TOKEN_PLUS,
};

/* REGEX COMPILER */
enum rex_opcode_e
{
    REX_INST_CHAR,             /* Match a codepoint range. Halt on mismatch */
    REX_INST_CHARSET_OR,       /* Set CHARSET_BIT if match codepoint range */
    REX_INST_CHARSET_TEST_IMM, /* Continue if match codepoint range. */
    REX_INST_CHARSET_TEST,     /* Continue if CHARSET_BIT is set. */
    REX_INST_CHARSET_NTEST,    /* Continue if CHARSET_BIT is unset. */
    REX_INST_MATCH,            /* Halt with success status */ 
    REX_INST_SPLIT,            /* Start new thread at x */
    REX_INST_JUMP              /* Jump this thread to x */
};

typedef enum rex_parse_token_e rex_token_t;
typedef enum rex_opcode_e rex_opcode_t;
typedef enum rex_mcset_fsm_state_e rex_mcset_fsm_state_t;
typedef enum rex_mcset_fsm_range_state_e rex_mcset_fsm_range_state_t;
typedef enum rex_escape_seq_mask_e rex_escape_seq_mask_t;
typedef enum rex_charset_fsm_state_e rex_charset_fsm_state_t;
typedef enum rex_utf8_codepoint_fsm_state_e rex_utf8_codepoint_fsm_state_t;

typedef struct rex_compiler_s rex_compiler_t;
typedef struct rex_instruction_s rex_instruction_t;

struct rex_instruction_s
{
    REX_SIZE_T x, y;
    rex_opcode_t op;
};

struct rex_compiler_s{
    void * mem;
    REX_SIZE_T mem_sz;
};

REX_SIZE_T
rex_parse_charset(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    rex_instruction_t *o_inst, REX_SIZE_T * o_inst_sz
);

REX_SIZE_T 
rex_parse_utf8_codepoint(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    uint32_t * const restrict cp
);

REX_SIZE_T
rex_parse_escape_seq(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    uint32_t * const restrict esc_seq,
    rex_instruction_t *o_inst, REX_SIZE_T * o_inst_sz
);

REX_SIZE_T
rex_parse_multichar_set(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    rex_instruction_t *o_inst, REX_SIZE_T * o_inst_sz
);


REX_SIZE_T
rex_parse_mcset_range(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    uint32_t *cp0, uint32_t *cp1 /* NONNULL */
);

REX_SIZE_T
rex_parse_token(
    const unsigned char * const restrict str,
    const REX_SIZE_T str_sz,
    rex_token_t * const restrict token
);

REX_SIZE_T 
rex_parse_charset_str(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    rex_instruction_t *o_inst, REX_SIZE_T * o_inst_sz
);
REX_SIZE_T
rex_compile_size(
    const unsigned char * i_regex_str,
    const REX_SIZE_T i_regex_str_sz,
    const rex_token_t tok
);

static const REX_SIZE_T rex_child_node_lut[] =
{
    /* OUT    In                            */
    0,     /* REX_PARSE_TOKEN_NULL          */
    0,     /* REX_PARSE_TOKEN_CHARSET_STR   */
    0,     /* REX_PARSE_TOKEN_RPAREN        */       
    1,     /* REX_PARSE_TOKEN_LPAREN        */
    2,     /* REX_PARSE_TOKEN_ALTERNATION   */
    2,     /* REX_PARSE_TOKEN_CONCAT        */
    1,     /* REX_PARSE_TOKEN_KLEEN         */
    2,     /* REX_PARSE_TOKEN_QUESTION      */
    1,     /* REX_PARSE_TOKEN_PLUS          */
};

static const REX_SIZE_T rex_compiled_digit_set_sz = 1;
static const REX_SIZE_T rex_compiled_digit_inv_set_sz = 2;
static const REX_SIZE_T rex_compiled_whitespace_set_sz = 2;
static const REX_SIZE_T rex_compiled_whitespace_inv_set_sz = 3;
static const REX_SIZE_T rex_compiled_word_set_sz = 4;
static const REX_SIZE_T rex_compiled_word_set_inv_sz = 5;

static const rex_instruction_t rex_compiled_digit_set[] =
{
    {'0', '9', REX_INST_CHARSET_OR}, 
};
static const rex_instruction_t rex_compiled_digit_inv_set[] =
{
    {REX_CHAR_MIN, '0'-1, REX_INST_CHARSET_OR}, 
    {'9'+1, REX_CHAR_MAX, REX_INST_CHARSET_OR} 
};
static const rex_instruction_t rex_compiled_whitespace_set[] =
{
    {'\t', '\r', REX_INST_CHARSET_OR}, /* \t \n \v \f \r */
    {' ', ' ', REX_INST_CHARSET_OR}, 
};
static const rex_instruction_t rex_compiled_whitespace_inv_set[] =
{
    {REX_CHAR_MIN, '\t'-1, REX_INST_CHARSET_OR},
    {'\r'+1, ' '-1, REX_INST_CHARSET_OR},
    {' '+1, REX_CHAR_MAX, REX_INST_CHARSET_OR}, 
};
static const rex_instruction_t rex_compiled_word_set[] =
{
    {'a', 'z', REX_INST_CHARSET_OR},
    {'A', 'Z', REX_INST_CHARSET_OR}, 
    {'0', '9', REX_INST_CHARSET_OR}, 
    {'_', '_', REX_INST_CHARSET_OR}, 
};
static const rex_instruction_t rex_compiled_word_inv_set[] =
{
    {REX_CHAR_MIN, '0'-1, REX_INST_CHARSET_OR},
    {'9'+1, 'A'-1, REX_INST_CHARSET_OR}, 
    {'Z'+1, '_'-1, REX_INST_CHARSET_OR}, 
    {'_'+1, 'a'-1, REX_INST_CHARSET_OR},
    {'z'+1, REX_CHAR_MAX, REX_INST_CHARSET_OR}, 
};

REX_BOOL_T 
rex_iswhitespace(const uint32_t cp);

REX_BOOL_T 
rex_isreserved(const uint32_t cp);

REX_BOOL_T 
rex_iswhitespace(const uint32_t cp)
{
    switch (cp)
    {
    case ' ':
    case '\n':
    case '\t':
    case '\v':
    case '\f':
    case '\r':
        return 1;
    default:
        return 0;
    }
}

REX_BOOL_T 
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

REX_SIZE_T 
rex_parse_utf8_codepoint(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    uint32_t * const restrict cp
)
{
    rex_utf8_codepoint_fsm_state_t state;
    uint32_t l = 0, i, j;
    state = REX_TRY(!!str && len > 0, REX_UTF8_FSM_DECODE);

    for(;;) switch (state)
    {
    case REX_UTF8_FSM_FAIL:
        return 0;
    case REX_UTF8_FSM_VALID_FIRST_BYTE:
        *cp |=((uint32_t)(((str[l] << i) & 0xFF)>> i)) << ((i-1)*6);
        j = 1;
        state = REX_TRY(l<len, REX_UTF8_FSM_CHAR_TAIL);
        l++;
        break;
    case REX_UTF8_FSM_CHAR_TAIL:
        state = REX_TRY(str[l] >> 6 == 2 && l<len, REX_UTF8_FSM_VALID_TAIL_BYTE);
        break;
    case REX_UTF8_FSM_SUCCESS:
        return l;
    case REX_UTF8_FSM_DECODE:
        state = REX_NCHOOSE((str[l] >> 7), REX_UTF8_FSM_ASCII_CHAR); /* OR REX_UTF8_FSM_CHAR */
        i = 1;
        j = 0;
        *cp = 0;
        break;
    case REX_UTF8_FSM_ASCII_CHAR:
        *cp = str[l];
        l++;
        return l;
    case REX_UTF8_FSM_CHAR:
        i = REX_NCHOOSE(((str[l] << i) & 0xFF)>> 7, i);
        state = REX_NCHOOSE(((str[l] << i) & 0xFF)>> 7, REX_UTF8_FSM_CHAR);
        break;
    case REX_UTF8_FSM_CHECK_BYTE_ONE:
        state = REX_TRY((i > 1 && i < 5), REX_UTF8_FSM_VALID_FIRST_BYTE);
        break;
    case REX_UTF8_FSM_VALID_TAIL_BYTE:
        *cp |=(uint32_t)(str[l] & 0x3F) << ((i-1-j) * 6);
        j++;
        l++;
        state = REX_CHOOSE(j<i, REX_UTF8_FSM_CHAR_TAIL);
        break;
    }
}


REX_SIZE_T
rex_parse_charset(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    rex_instruction_t *o_inst, REX_SIZE_T * o_inst_sz
){
    uint32_t  cp = 0;
    REX_SIZE_T l = 0;
    rex_instruction_t out;
    rex_charset_fsm_state_t state;
    state = REX_TRY(!!str, REX_CS_FSM_ALPHABET_SET);
    if (o_inst_sz) *o_inst_sz = 0;

    for(;;) switch (state)
    {
    case REX_CS_FSM_INST_ISNULL:
        state = REX_CHOOSE(o_inst!=NULL && l!=0, REX_CS_FSM_SET_INST);
        break;
    case REX_CS_FSM_SET_INST:
        *o_inst = out;
    /*FALLTHROUGH*/
    case REX_CS_FSM_EXIT:
        return l;
    case REX_CS_FSM_ALPHABET_SET:
        l = rex_parse_utf8_codepoint(str, len, &cp);
        state = REX_TRY(l!=0 && cp != '.' && cp != 0, REX_CS_FSM_ESC_SEQ);
        out.op = REX_INST_CHARSET_TEST_IMM;
        out.x = REX_CHAR_MIN;
        out.y = REX_CHAR_MAX;
        if (o_inst_sz && l) *o_inst_sz = 1;
        break;
    case REX_CS_FSM_ESC_SEQ:
        l = rex_parse_escape_seq(str, len, &cp, o_inst, o_inst_sz);
        state = REX_TRY(!l, REX_CS_FSM_MULTICHAR_SET);
        out.op = REX_INST_CHARSET_TEST;
        break;
    case REX_CS_FSM_MULTICHAR_SET:
        l = rex_parse_multichar_set(str, len, o_inst, o_inst_sz);
        state = REX_TRY(!l, REX_CS_FSM_SINGLECHAR_SET);
        break;
    case REX_CS_FSM_SINGLECHAR_SET:
        l = rex_parse_utf8_codepoint(str, len, &cp);
        l = REX_TRY((cp != 0), l);
        l = REX_TRY(!rex_isreserved(cp), l);
        out.op = REX_INST_CHARSET_TEST_IMM;
        out.x = cp;
        out.y = cp;
        state = REX_TRY(l == 0, REX_CS_FSM_EXIT);
        if (o_inst_sz && l) *o_inst_sz = 1;
        break;
    }

}

REX_SIZE_T
rex_parse_escape_seq(
    const unsigned char * const restrict i_str,
    REX_SIZE_T i_len,
    uint32_t * const restrict esc_seq,
    rex_instruction_t *o_inst, REX_SIZE_T * o_inst_sz
)
{
    REX_SIZE_T l, i, j;
    uint32_t e, seq;
    const rex_instruction_t *precomp;
    rex_instruction_t out;

    l = rex_parse_utf8_codepoint(i_str, i_len, &e);
    e *= (l!=0);
    switch(e)
    {
    case '\\':
        i = rex_parse_utf8_codepoint(i_str+l, i_len-l, &e);
        l += i;
        l = REX_TRY(i!= 0, l);
        break;
    default:
       return 0;
    }
    switch(e)
    {
    case 'w':
        seq = REX_ESC_SEQ_w;
        precomp = rex_compiled_word_set;
        i = rex_compiled_word_set_sz;
        break;
    case 'W':
        seq = REX_ESC_SEQ_W; 
        precomp = rex_compiled_word_set;
        i = rex_compiled_word_set_sz;
        break;
    case 's':
        seq = REX_ESC_SEQ_s; 
        precomp = rex_compiled_whitespace_set;
        i  = rex_compiled_whitespace_set_sz;
        break;
    case 'S':
        seq = REX_ESC_SEQ_S; 
        precomp = rex_compiled_whitespace_set;
        i = rex_compiled_whitespace_set_sz;
        break;
    case 'd':
        seq = REX_ESC_SEQ_d; 
        precomp = rex_compiled_digit_set;
        i = rex_compiled_digit_set_sz;
        break;
    case 'D':
        seq = REX_ESC_SEQ_D; 
        precomp = rex_compiled_digit_set;
        i = rex_compiled_digit_set_sz;
        break;
    case 0:
        return l;
    default:
        seq = e;
        precomp = &out;
        out.op = REX_INST_CHARSET_OR;
        out.x = e;
        out.y = e;
        i = 1;
        break;
    }
    if (o_inst_sz) *o_inst_sz += i;
    if (o_inst) for(j = 0; j < i; j++) o_inst[j] = precomp[j];
    if (e) *esc_seq = seq;
    
    return l;
}


REX_SIZE_T
rex_parse_multichar_set(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    rex_instruction_t *o_inst, REX_SIZE_T * o_inst_sz

){

    uint32_t i = 0, cp, l = 0; 
    REX_SIZE_T scrap = 0;
    REX_BOOL_T inv;
    uint32_t rcp0, rcp1;
    rex_mcset_fsm_state_t state, emit_ret;
    rex_instruction_t out;

    if (!o_inst_sz) o_inst_sz = &scrap;
    *o_inst_sz = 0;

    l = rex_parse_utf8_codepoint(str, len, &cp);
    cp *= (l!=0);
    switch(cp)
    {
    case '[':
        i = rex_parse_utf8_codepoint(str+l, len-l, &cp);
        cp = REX_TRY(i!=0, cp);
        break;
    default:
       return 0;
    }

    state = REX_NCHOOSE(i==0, REX_MCSET_FSM_BODY);
    switch (cp)
    {
    case '^':
        inv = 1;
        l+=i;
        i = rex_parse_utf8_codepoint(str+l, len-l, &cp);
        l = REX_TRY(i!= 0, l);
        state = REX_NCHOOSE(i==0 || cp==']', REX_MCSET_FSM_BODY);
        l += REX_TRY(cp==']', i);
        break;
    case 0:
        return 0;
    default:
        inv = 0;
        break;
    }

    for(;;) switch (state)
    {
    case REX_MCSET_FSM_BODY:
        emit_ret = REX_CHOOSE(cp==']' || cp == 0, REX_MCSET_FSM_EXIT);
        state = REX_CHOOSE(cp != 0, REX_MCSET_FSM_EMIT);
        l += REX_TRY(cp==']', i);
        l = REX_TRY(cp != 0, l);
        out.op = REX_NCHOOSE(inv, REX_INST_CHARSET_TEST);
        break;
    case REX_MCSET_FSM_EXIT:
        return l;
    case REX_MCSET_FSM_RANGE:
        i = rex_parse_mcset_range(str+l, len-l, &rcp0, &rcp1);
        state = REX_NCHOOSE(i==0, REX_MCSET_FSM_RANGE_ORDERED);
        l += i;
        break;
    case REX_MCSET_FSM_RANGE_ORDERED:
        l = REX_TRY(rcp0 <= rcp1, l);
        emit_ret = REX_NCHOOSE(l==0, REX_MCSET_FSM_BODY);
        out.op = REX_INST_CHARSET_OR;
        out.x = rcp0;
        out.y = rcp1;
        state = REX_CHOOSE(o_inst == NULL, REX_MCSET_FSM_EMIT);
        break;
    case REX_MCSET_FSM_ESC:
        i = rex_parse_escape_seq(str+l, len-l, &cp, o_inst, o_inst_sz);
        state = REX_TRY(i==0, REX_MCSET_FSM_CHAR);
        l += i;
        break;
    case REX_MCSET_FSM_CHAR:
        i = rex_parse_utf8_codepoint(str+l, len-l, &cp);
        l += i;
        l = REX_TRY(i!=0, l);
        cp = REX_TRY(i!=0, cp);
        emit_ret = REX_NCHOOSE(i==0 || cp == '\\' || cp ==']', REX_MCSET_FSM_BODY);
        state = REX_CHOOSE(i==0 || cp == '\\' || cp == ']', REX_MCSET_FSM_EMIT);
        out.x = cp;
        out.y = cp;
        out.op = REX_INST_CHARSET_OR;
        break;
    case REX_MCSET_FSM_EMIT:
        o_inst[(*o_inst_sz)++]=out;
    /*FALLTHROUGH*/
    case REX_MCSET_FSM_EMIT_RETURN:
        state = emit_ret;
        break;
    }
}

REX_SIZE_T
rex_parse_mcset_range(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    uint32_t *cp0, uint32_t *cp1 /* NONNULL */
)
{

    REX_SIZE_T l =0, i=0; 
    uint32_t *cp;
    rex_mcset_fsm_range_state_t state, char_exit;

    state = REX_MCSET_RANGE_ESC;
    cp = cp0;
    char_exit = REX_MCSET_RANGE_HYPHEN;
    
    for(;;) switch (state)
    {
    case REX_MCSET_RANGE_EXIT:
        return l;
    case REX_MCSET_RANGE_ESC:
        i = rex_parse_escape_seq(str+l,len-l, cp, NULL, 0);
        state = (i!=0) + REX_MCSET_RANGE_CHAR;
        l += i;
        l = REX_TRY(state != 0, l);
        break;
    case REX_MCSET_RANGE_CHAR:
        i = rex_parse_utf8_codepoint(str+l, len-l, cp);
        l += i;
        l = REX_TRY(i!=0 && *cp!=0 && *cp!=']' && *cp!='\\', l);
        state = REX_TRY(l!=0, char_exit);
        break;
    case REX_MCSET_RANGE_ESC_ISVALID:
        state = REX_TRY(*cp != 0 && (*cp & 0xFF000000) == 0, char_exit);
        l = REX_TRY(state!=0, l);
        break;
    case REX_MCSET_RANGE_HYPHEN:
        cp = cp1;
        i = rex_parse_utf8_codepoint(str+l, len-l, cp);
        l += i;
        l = REX_TRY(i!=0 && *cp=='-', l);
        state = REX_TRY(l!=0, REX_MCSET_RANGE_ESC);
        char_exit = REX_MCSET_RANGE_EXIT;
        break;
    }
}
REX_SIZE_T 
rex_parse_charset_str(
    const unsigned char * const restrict str,
    REX_SIZE_T len,
    rex_instruction_t *o_inst, REX_SIZE_T * o_inst_sz
)
{
    REX_SIZE_T cpi, insti, insti_temp, cpi_temp;
    
    cpi = 0;
    insti =0;
    do {
        cpi_temp = rex_parse_charset(
            str + cpi, 
            len - cpi,
            o_inst + insti,
            &insti_temp
        );

        cpi += cpi_temp;
        insti += insti_temp;
    }while(cpi_temp);
    if(o_inst_sz) *o_inst_sz = insti;

    return cpi;
}

int
rex_compile_regex(
    const unsigned char * i_regex_str,
    const REX_SIZE_T i_regex_str_sz,
    rex_instruction_t * const o_inst,
    REX_SIZE_T * const io_inst_sz,
    rex_compiler_t * io_compiler
)
{
    int r;
    REX_SIZE_T i, j, k;
    REX_SIZE_T stri, stack_sz;
    rex_token_t tok, tok_tmp;
    uint8_t * const stack = io_compiler->mem;
    uint8_t * bottom = stack + io_compiler->mem_sz-1;

    stri = 0;
    stack_sz = 0;

    /* Check if output has enough space */
    i = 0;
    j = 0;
    *io_inst_sz = 0;
    do
    {
        j = rex_parse_token(
            i_regex_str + i,
            i_regex_str_sz - i,
            &tok
        );
        k = rex_compile_size(
            i_regex_str + i,
            j,
            tok
        );
        i += j;
        *io_inst_sz += k;
    }while (j != 0);

    if (i_regex_str_sz - i != 0 && i_regex_str[i] != 0) return 2;
    if (!o_inst) return 0;
    
    /* Start Compilation */

    /* Make Tree */
    for (stri = 0; stri < i_regex_str_sz && i_regex_str[stri]; stri+=i)
    {
        i = rex_parse_token(i_regex_str+stri, i_regex_str_sz, &tok);
        if (!i) return 0;

    shunting_start:
        switch(tok)
        {
        case REX_PARSE_TOKEN_CHARSET_STR:
            /* EMIT TOK */
            if (stack + stack_sz >= bottom + i + 1) return 1;
            *bottom = 0;
            bottom--;
            memcpy(bottom - i, i_regex_str+stri, i);
            bottom -= i;
            break;
        case REX_PARSE_TOKEN_LPAREN:
            /* PUSH TO OPSTACK */
            if (stack + stack_sz >= bottom) return 1;
            stack[stack_sz++] = tok;
            break;
        case REX_PARSE_TOKEN_RPAREN:
            for (;tok != REX_PARSE_TOKEN_LPAREN;)
            {
                if(stack_sz == 0) return 2;
                if (stack + stack_sz >= bottom + 1) return 1;
                /* EMIT TOP OF OPSTACK*/
                *bottom = stack[stack_sz];
                /* POP OPSTACK*/
                stack_sz--;
            }
            break;
        default:
            /* WHILE STACK_TOP PRECEDENCE > TOK */
            if (stack_sz) while(stack[stack_sz-1] > tok)
            {
                if (stack + stack_sz >= bottom + 1) return 1;
                /* EMIT TOP OF OPSTACK*/
                *bottom = stack[stack_sz];
                /* POP OPSTACK*/
                stack_sz--;
            }
            /* PUSH TOK TO OPSTACK */
            if (stack + stack_sz >= bottom) return 1;
            stack[stack_sz++] = tok;
            
            break;
        } 
        /* Handle Implicit Concat Using Lookahead */
        rex_parse_token(i_regex_str+stri+i, i_regex_str_sz, &tok_tmp);
        switch(tok)
        {
        case REX_PARSE_TOKEN_PLUS:
        case REX_PARSE_TOKEN_KLEEN:
        case REX_PARSE_TOKEN_QUESTION:
        case REX_PARSE_TOKEN_RPAREN:
            switch (tok_tmp)
            {
            case REX_PARSE_TOKEN_CHARSET_STR:
            case REX_PARSE_TOKEN_LPAREN:
                /* Prevent tree mangling due to implicit concat */
                tok = REX_PARSE_TOKEN_CONCAT;
                goto shunting_start;
                if (r) return r;
            default:
                break;
            }
        default:
            break;
        }
    }

    for(;;)
    {
        /* EMIT TOP OF OPSTACK*/
        if (stack + stack_sz >= bottom + 1) return 1;
        /* EMIT TOP OF OPSTACK*/
        *bottom = stack[stack_sz];
        /* POP OPSTACK*/
        stack_sz--;
    }

    return 0;
}

REX_SIZE_T
rex_compile_size(
    const unsigned char * i_regex_str,
    const REX_SIZE_T i_regex_str_sz,
    const rex_token_t tok
)
{
    REX_SIZE_T cpi = 0; 
    REX_SIZE_T insti = 0; 
    switch (tok)
    {
    case REX_PARSE_TOKEN_PLUS:
        return 1;
    case REX_PARSE_TOKEN_NULL:
        return 0;
    case REX_PARSE_TOKEN_KLEEN:
        return 2;
    case REX_PARSE_TOKEN_QUESTION:
        return 1;
    case REX_PARSE_TOKEN_LPAREN:
        return 1; /* TODO: */
    case REX_PARSE_TOKEN_CONCAT:
        return 0;
    case REX_PARSE_TOKEN_ALTERNATION:
        return 2;
    case REX_PARSE_TOKEN_CHARSET_STR:
        cpi = rex_parse_charset_str(i_regex_str, i_regex_str_sz, NULL, &insti);
        if (cpi) 
            return insti;
        /*FALLTHROUGH */
    default:
        /*ERROR*/
        return -1;
    }
}

REX_SIZE_T
rex_parse_token(
    const unsigned char * const restrict str,
    const REX_SIZE_T str_sz,
    rex_token_t * const restrict token
)
{
    REX_SIZE_T i;
    switch(str[0])
    {
    case '(':
        *token = REX_PARSE_TOKEN_LPAREN;
        return 1;
    case ')' :
        *token = REX_PARSE_TOKEN_RPAREN;
        return 1;
    case '*':
        *token = REX_PARSE_TOKEN_KLEEN;
       return 1;
    case '+':
       *token = REX_PARSE_TOKEN_PLUS;
       return 1;
    case '|':
       *token = REX_PARSE_TOKEN_ALTERNATION;
       return 1;
    case 0:
       return 0;
    default:
        break;
    }
    
    i = rex_parse_charset_str(str, str_sz, 0, NULL);
    if (i) *token = REX_PARSE_TOKEN_CHARSET_STR;
    return i;
}

#endif /* REX_IMPLMENTATION */

