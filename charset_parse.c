#include <stdint.h>

/* 
 * TODO:
 *
 *escape sequences
 *
 */
enum utf8_codepoint_fsm_state_e
{
    CS_FSM_UTF8_FAIL = 0,
    CS_FSM_UTF8_VALID_FIRST_BYTE,
    CS_FSM_UTF8_CHAR_TAIL,
    CS_FSM_UTF8_SUCCESS,
    CS_FSM_UTF8_DECODE,
    CS_FSM_ASCII_CHAR,
    CS_FSM_UTF8_CHAR,
    CS_FSM_UTF8_CHECK_BYTE_ONE,
    CS_FSM_UTF8_VALID_TAIL_BYTE,

};

enum charset_fsm_state_e
{
    CS_FSM_FAIL = 0,
    CS_FSM_SUCCESS,
    CS_FSM_CHECK_BSLASH,
    CS_FSM_CP_DECODE,
    CS_HANDLE_BSLASH,
    CS_HANDLE_NOTSLASH,
    CS_FSM_STR_NONNULL,
    CS_FSM_EXPECT_LBRACKET,
    CS_FSM_CHECK_NEGATE,
    CS_FSM_BYTES_LEFT,
    CS_FSM_BODY,
    CS_FSM_NEGATED
};
typedef enum charset_fsm_state_e charset_fsm_state_t;
typedef enum utf8_codepoint_fsm_state_e utf8_codepoint_fsm_state_t;


uint32_t 
parse_utf8_codepoint(
    const unsigned char * const restrict str,
    uint32_t * const restrict cp
);
uint32_t 
parse_utf8_codepoint(
    const unsigned char * const restrict str,
    uint32_t * const restrict cp
)
{
    utf8_codepoint_fsm_state_t state;
    uint32_t l = 0, i, j;
    state = (!!str) * CS_FSM_UTF8_DECODE;
top:
    switch (state)
    {
    case CS_FSM_UTF8_FAIL:
        return 0;
    case CS_FSM_UTF8_VALID_FIRST_BYTE:
        *cp |=((uint32_t)(((str[l] << i) & 0xFF)>> i)) << ((i-1)*6);
        j = 1;
        l++;
    /*FALLTHROUGH*/
    case CS_FSM_UTF8_CHAR_TAIL:
        state = (str[l] >> 6 == 2) * CS_FSM_UTF8_VALID_TAIL_BYTE;
        goto top;
    case CS_FSM_UTF8_SUCCESS:
        return l;
    case CS_FSM_UTF8_DECODE:
        state = (str[l] >> 7) + CS_FSM_ASCII_CHAR; /* OR CS_FSM_UTF8_CHAR */
        i = 1;
        j = 0;
        *cp = 0;
        goto top;
    case CS_FSM_ASCII_CHAR:
        *cp = str[l];
        l++;
        return l;
    case CS_FSM_UTF8_CHAR:
        i += (((str[l] << i) & 0xFF)>> 7);
        state = (!(((str[l] << i) & 0xFF)>> 7)) + CS_FSM_UTF8_CHAR;
        goto top;
    case CS_FSM_UTF8_CHECK_BYTE_ONE:
        state = (i > 1 && i < 5) * CS_FSM_UTF8_VALID_FIRST_BYTE;
        goto top;
    case CS_FSM_UTF8_VALID_TAIL_BYTE:
        *cp |=(uint32_t)(str[l] & 0x3F) << ((i-1-j) * 6);
        j++;
        l++;
        state = (j>=i) + CS_FSM_UTF8_CHAR_TAIL;
        goto top;
    }
}


uint32_t
parse_charset(
    const unsigned char * const restrict str
);

uint32_t
parse_charset(
    const unsigned char * const restrict str
){
    uint32_t i, cp, l = 0;
    charset_fsm_state_t state, cp_exit;
    state = (!!str) * CS_FSM_STR_NONNULL;
    uint8_t negated;
top:
    switch (state)
    {
    case CS_FSM_FAIL:
        return 0;
    case CS_FSM_SUCCESS:
        return l;
    case CS_FSM_CHECK_BSLASH:
        state = ( cp != '\\') + CS_HANDLE_BSLASH;
        goto top;
    case CS_FSM_CP_DECODE:
        i = parse_utf8_codepoint(str + l, &cp);
        l+=i;
        state = (!!i) * cp_exit;
        goto top;
    case CS_FSM_STR_NONNULL:
        state = CS_FSM_CP_DECODE;
        cp_exit = CS_FSM_EXPECT_LBRACKET;
        goto top;
    case CS_HANDLE_BSLASH:
        /*TODO: Not implemented*/
        state=CS_FSM_FAIL;
        goto top;
    case CS_HANDLE_NOTSLASH:
        /*TODO: Not implemented*/
        state=CS_FSM_FAIL;
        goto top;
    case CS_FSM_EXPECT_LBRACKET:
        state = (cp == '[') * CS_FSM_CP_DECODE;
        cp_exit = CS_FSM_CHECK_NEGATE; 
        goto top;
    case CS_FSM_CHECK_NEGATE:
        negated = cp=='^';
        state = negated + CS_FSM_BODY;
        goto top;
    case CS_FSM_BYTES_LEFT:
        state = (cp != ']') + CS_FSM_SUCCESS;
        goto top;
    case CS_FSM_BODY:
        state = (cp != 0) * CS_FSM_BYTES_LEFT;
        goto top;
    case CS_FSM_NEGATED:
        cp_exit = CS_FSM_BODY;
        state = CS_FSM_CP_DECODE;
        goto top;
    }
}

