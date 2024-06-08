#include <stdint.h>
#include <stddef.h>

enum utf8_codepoint_fsm_state_e
{
    UTF8_FSM_FAIL = 0,
    UTF8_FSM_VALID_FIRST_BYTE,
    UTF8_FSM_CHAR_TAIL,
    UTF8_FSM_SUCCESS,
    UTF8_FSM_DECODE,
    UTF8_FSM_ASCII_CHAR,
    UTF8_FSM_CHAR,
    UTF8_FSM_CHECK_BYTE_ONE,
    UTF8_FSM_VALID_TAIL_BYTE,

};
enum mcset_fsm_state_e
{
    MCSET_FSM_BODY = 0,
    MCSET_FSM_EXIT,
    MCSET_FSM_RANGE,
    MCSET_FSM_RANGE_ORDERED,
    MCSET_FSM_ESC,
    MCSET_FSM_CHAR
};

enum charset_fsm_state_e
{
    CS_FSM_DERIV_ISNULL =0,
    CS_FSM_SET_DERIV,
    CS_FSM_EXIT ,
    CS_FSM_ALPHABET_SET,
    CS_FSM_ESC_SEQ,
    CS_FSM_MULTICHAR_SET,
    CS_FSM_SINGLECHAR_SET
};

enum escape_seq_mask_e
{
    ESC_SEQ_w = 1 << (0 + 24),
    ESC_SEQ_W = 1 << (1 + 24),
    ESC_SEQ_s = 1 << (2 + 24),
    ESC_SEQ_S = 1 << (3 + 24),
    ESC_SEQ_d = 1 << (4 + 24),
    ESC_SEQ_D = 1 << (5 + 24)
};

enum mcset_fsm_range_state_e
{
    MCSET_RANGE_EXIT = 0,
    MCSET_RANGE_ESC,
    MCSET_RANGE_CHAR,   
    MCSET_RANGE_ESC_ISVALID,
    MCSET_RANGE_HYPHEN,
};

typedef enum mcset_fsm_state_e mcset_fsm_state_t;
typedef enum mcset_fsm_range_state_e mcset_fsm_range_state_t;
typedef enum escape_seq_mask_e escape_seq_mask_t;
typedef enum charset_fsm_state_e charset_fsm_state_t;
typedef enum utf8_codepoint_fsm_state_e utf8_codepoint_fsm_state_t;

static const char * const empty_str = "";
static const char * const nullable_lut[2]= 
{
    NULL, empty_str
};

uint32_t 
parse_utf8_codepoint(
    const unsigned char * const restrict str,
    uint32_t * const restrict cp
);

uint32_t
parse_escape_seq(
    const unsigned char * const restrict str,
    uint32_t * const restrict esc_seq,
    const char dparam, const char ** derive
);

uint32_t
parse_multichar_set(
    const unsigned char * const restrict str,
    const char dparam, const char ** derive
);


uint32_t
parse_mcset_range(
    const unsigned char * const restrict str,
    uint32_t *cp0, uint32_t *cp1 /* NONNULL */
);

uint8_t 
iswhitespace(const uint32_t cp);

uint8_t 
isreserved(const uint32_t cp);

uint8_t 
iswhitespace(const uint32_t cp)
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

uint8_t 
isreserved(const uint32_t cp)
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
uint32_t 
parse_utf8_codepoint(
    const unsigned char * const restrict str,
    uint32_t * const restrict cp
)
{
    utf8_codepoint_fsm_state_t state;
    uint32_t l = 0, i, j;
    state = (!!str) * UTF8_FSM_DECODE;
top:
    switch (state)
    {
    case UTF8_FSM_FAIL:
        return 0;
    case UTF8_FSM_VALID_FIRST_BYTE:
        *cp |=((uint32_t)(((str[l] << i) & 0xFF)>> i)) << ((i-1)*6);
        j = 1;
        l++;
    /*FALLTHROUGH*/
    case UTF8_FSM_CHAR_TAIL:
        state = (str[l] >> 6 == 2) * UTF8_FSM_VALID_TAIL_BYTE;
        goto top;
    case UTF8_FSM_SUCCESS:
        return l;
    case UTF8_FSM_DECODE:
        state = (str[l] >> 7) + UTF8_FSM_ASCII_CHAR; /* OR UTF8_FSM_CHAR */
        i = 1;
        j = 0;
        *cp = 0;
        goto top;
    case UTF8_FSM_ASCII_CHAR:
        *cp = str[l];
        l++;
        return l;
    case UTF8_FSM_CHAR:
        i += (((str[l] << i) & 0xFF)>> 7);
        state = (!(((str[l] << i) & 0xFF)>> 7)) + UTF8_FSM_CHAR;
        goto top;
    case UTF8_FSM_CHECK_BYTE_ONE:
        state = (i > 1 && i < 5) * UTF8_FSM_VALID_FIRST_BYTE;
        goto top;
    case UTF8_FSM_VALID_TAIL_BYTE:
        *cp |=(uint32_t)(str[l] & 0x3F) << ((i-1-j) * 6);
        j++;
        l++;
        state = (j>=i) + UTF8_FSM_CHAR_TAIL;
        goto top;
    }
}


uint32_t
parse_charset(
    const unsigned char * const restrict str,
    uint32_t const dparam, const char ** derive
){
    uint32_t  cp = 0, l = 0;
    const char *d;
    charset_fsm_state_t state;
    state = (!!str) * CS_FSM_ALPHABET_SET;
top:
    switch (state)
    {

    case CS_FSM_DERIV_ISNULL:
        state = CS_FSM_SET_DERIV + (derive == NULL);
        goto top;
    case CS_FSM_SET_DERIV:
        *derive = d;
    /*FALLTHROUGH*/
    case CS_FSM_EXIT:
        return l;
    case CS_FSM_ALPHABET_SET:
        l = parse_utf8_codepoint(str, &cp);
        state = (l!=0 && cp != '.') * CS_FSM_ESC_SEQ;
        d=empty_str;
        goto top;
    case CS_FSM_ESC_SEQ:
        l = parse_escape_seq(str, &cp, dparam, &d);
        state = (!l) * CS_FSM_MULTICHAR_SET;
        goto top;
    case CS_FSM_MULTICHAR_SET:
        l = parse_multichar_set(str, dparam, &d);
        state = (!l) * CS_FSM_SINGLECHAR_SET;
        goto top;
    case CS_FSM_SINGLECHAR_SET:
        l = parse_utf8_codepoint(str, &cp);
        l *= (cp != 0);
        l *= (!isreserved(cp));
        d = nullable_lut[cp==dparam && l!=0];
        state = (l== 0) * CS_FSM_EXIT;
        goto top;
    }

}

uint32_t
parse_escape_seq(
    const unsigned char * const restrict str,
    uint32_t * const restrict esc_seq,
    const char dparam, const char ** derive
)
{
    uint32_t  l, i, e;
    uint8_t cond;
    const char * d;
    const char ** dlut[2] = {&d, derive};
    l = parse_utf8_codepoint(str, &e);
    e *= (l!=0);
    *dlut[derive!=NULL] = NULL;
    switch(e)
    {
    case '\\':
        i = parse_utf8_codepoint(str+l, &e);
        l += i;
        l *= (i!= 0);
        break;
    default:
       return 0;
    }
    switch(e * (esc_seq != 0))
    {
    case 'w':
        *esc_seq = ESC_SEQ_w;

        cond = dparam >= '0' && dparam <= '9';
        cond |= dparam >= 'a' && dparam <= 'z';
        cond |= dparam >= 'A' && dparam <= 'Z';
        cond |= dparam == '_';
        break;
    case 'W':
        *esc_seq = ESC_SEQ_W; 
        cond = dparam < '0' && dparam > '9';
        cond &= dparam < 'a' && dparam > 'z';
        cond &= dparam < 'A' && dparam > 'Z';
        cond &= dparam != '_';
        break;
    case 's':
        *esc_seq = ESC_SEQ_s; 
        cond = iswhitespace(dparam);
        break;
    case 'S':
        *esc_seq = ESC_SEQ_S; 
        cond = !iswhitespace(dparam);
        break;
    case 'd':
        *esc_seq = ESC_SEQ_d; 
        cond = dparam >= '0' && dparam <= '9';
        break;
    case 'D':
        *esc_seq = ESC_SEQ_D; 
        cond = dparam < '0' || dparam > '9';
        break;
    default:
        cond = dparam==e;
        break;
    }
    *dlut[derive!=NULL]  = nullable_lut[(cond && *derive==NULL)];
    return l;
}


uint32_t
parse_multichar_set(
    const unsigned char * const restrict str,
    const char dparam, const char ** derive
){

    uint32_t i = 0, cp, l = 0;
    uint8_t inv, cond = 0;
    uint32_t rcp0, rcp1;
    mcset_fsm_state_t state;
    const char * d;

    *derive = NULL;

    l = parse_utf8_codepoint(str, &cp);
    cp *= (l!=0);
    switch(cp)
    {
    case '[':
        i = parse_utf8_codepoint(str+l, &cp);
        cp *= (i!=0);
        break;
    default:
       return 0;
    }
    state = (i==0) + MCSET_FSM_BODY;

    switch (cp)
    {
    case '^':
        inv = 0;
        i = parse_utf8_codepoint(str+l, &cp);
        l *= (i!= 0);
        state = (i==0) + MCSET_FSM_BODY;
        l +=i;
        break;
    case 0:
        return 0;
    default:
        inv = 1;
        break;
    }

top:
    switch (state)
    {
    case MCSET_FSM_BODY:
        state = (cp!=']' && cp != 0) + MCSET_FSM_EXIT;
        l += i * (cp==']');
        l *= (cp != 0);
        goto top;
    case MCSET_FSM_EXIT:
        cond |= *derive!=NULL;
        *derive = nullable_lut[cond ^ inv];
        return l;
    case MCSET_FSM_RANGE:
        i = parse_mcset_range(str+l, &rcp0, &rcp1);
        state = (i==0) + MCSET_FSM_RANGE_ORDERED;
        l += i;
        goto top;
    case MCSET_FSM_RANGE_ORDERED:
        l *= (rcp0 <= rcp1);
        state = (l==0) + MCSET_FSM_BODY;
        cond = (dparam>=rcp0 && dparam <=rcp1 && state == MCSET_FSM_BODY);
        goto top;
    case MCSET_FSM_ESC:
        i = parse_escape_seq(str+l, NULL, dparam, &d);
        state = (i==0) * MCSET_FSM_CHAR;
        l += i;
        cond = (d!=NULL);
        goto top;
    case MCSET_FSM_CHAR:
        i = parse_utf8_codepoint(str+l, &cp);
        l += i;
        l*= (i!=0);
        cp *= (i!=0);
        state = (i==0 || cp == '\\' || cp ==']') + MCSET_FSM_BODY;
        cond =  (cp==dparam && *derive==NULL && state == MCSET_FSM_BODY);
        goto top;
    }
}

uint32_t
parse_mcset_range(
    const unsigned char * const restrict str,
    uint32_t *cp0, uint32_t *cp1 /* NONNULL */
)
{

    uint32_t l =0, i=0; 
    uint32_t *cp;
    mcset_fsm_range_state_t state, char_exit;

    state = MCSET_RANGE_ESC;
    cp = cp0;
    char_exit = MCSET_RANGE_HYPHEN;
top:
    switch (state)
    {
    case MCSET_RANGE_EXIT:
        return l;
    case MCSET_RANGE_ESC:
        i = parse_escape_seq(str+l, cp, 0, NULL);
        state = (i!=0) + MCSET_RANGE_CHAR;
        l += i;
        l *= (state != 0);
        goto top;
    case MCSET_RANGE_CHAR:
        i = parse_utf8_codepoint(str+l, cp);
        l += i;
        l *= i!=0 && *cp!=0 && *cp!=']' && *cp!='\\';
        state = (l!=0) * char_exit;
        goto top;
    case MCSET_RANGE_ESC_ISVALID:
        state = (*cp != 0 && (*cp & 0xFF000000) == 0) * char_exit;
        l *= (state!=0);
        goto top;
    case MCSET_RANGE_HYPHEN:
        cp = cp1;
        i = parse_utf8_codepoint(str+l, cp);
        l += i;
        l *= i!=0 && *cp=='-';
        state = (l!=0) * MCSET_RANGE_ESC;
        char_exit = MCSET_RANGE_EXIT;
        goto top;
    }
}

