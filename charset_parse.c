#include <stdint.h>

/* 
 * TODO:
 *
 *escape sequences
 *
 */
enum charset_fsm_cond_e
{
    CS_FSM_FAIL = 0,
    CS_FSM_SUCCESS,
    CS_FSM_ASCII_CHAR_NOT_BSLASH_RBRACKET,
    CS_FSM_RANGE,
    CS_FSM_BYTES_LEFT,
    CS_FSM_NO_TERMINATOR,
    CS_FSM_STR_NONNULL,
    CS_FSM_LBRACKET,
    CS_FSM_ASCII_CHAR,
    CS_FSM_UTF8_CHAR,
    CS_FSM_BSLASH,
    CS_FSM_ASCII_CHAR_NOT_BSLASH,
    CS_FSM_UTF8_BYTES_LEFT,
    CS_FSM_UTF8_CODEPOINT,
    CS_FSM_UTF8_VALID_BYTE
};
typedef enum charset_fsm_cond_e charset_fsm_cond_t;

uint32_t
parse_charset(
    const unsigned char * const restrict str
);

uint32_t
parse_charset(
    const unsigned char * const restrict str
){
    uint32_t i, j, l = 0;
    charset_fsm_cond_t condition;
    condition = (!!str) * CS_FSM_STR_NONNULL;

top:
    switch (condition)
    {
    case CS_FSM_FAIL:
        return 0;
    case CS_FSM_SUCCESS:
        return l;
    case CS_FSM_ASCII_CHAR_NOT_BSLASH_RBRACKET:
        condition = !(str[l+1] == '-');
        l += condition;
        condition += CS_FSM_RANGE;
        goto top;
    case CS_FSM_RANGE_PREFIX:
        condition = (str[l] <= str[l+2] && (str[l+2] >> 7 == 0)) * CS_FSM_BYTES_LEFT;
        l+=3;
        goto top;
    case CS_FSM_BYTES_LEFT:
        condition = (!!str[l]) * CS_FSM_NO_TERMINATOR;
        goto top;
    case CS_FSM_NO_TERMINATOR:
        condition = (str[l] >> 7) + CS_FSM_ASCII_CHAR;
        goto top;
    case CS_FSM_STR_NONNULL:
        condition = (str[l] == '[') * CS_FSM_LBRACKET;
        l++;
        goto top;    
    case CS_FSM_LBRACKET:
        l += (str[l] == '^');
        condition = CS_FSM_BYTES_LEFT;
        goto top;
    case CS_FSM_ASCII_CHAR:
        condition = (str[l] != '\\') + CS_FSM_BSLASH;
        goto top;
    case CS_FSM_UTF8_CHAR:
        for (i = 0; (str[l] << (i + 1)) & (1 << 7); i++);
        condition =  (i!=0) * CS_FSM_UTF8_BYTES_LEFT;
        l++;
        goto top;
    case CS_FSM_BSLASH:
        l++;
    case CS_FSM_ASCII_CHAR_NOT_BSLASH:
        condition = (str[l] != ']') + CS_FSM_SUCCESS;
        goto top;
    case CS_FSM_UTF8_BYTES_LEFT:
        condition = (((3 << 6) & str[l]) == (1<<7)) * CS_FSM_UTF8_VALID_BYTE;
        l++;
        goto top;
    case CS_FSM_UTF8_CODEPOINT:
        condition = CS_FSM_BYTES_LEFT;
        goto top;
    case CS_FSM_UTF8_VALID_BYTE:
        i--;
        condition = (i < 1) + CS_FSM_UTF8_BYTES_LEFT;
        goto top;
    }
}

