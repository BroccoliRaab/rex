#ifndef CHARSET_PARSE_H
#define CHARSET_PARSE_H

#include <stdint.h>

enum parse_tolken_e
{
    PARSE_TOKEN_NULL = 0,
    BRZO_EMPTY_SET,
    PARSE_TOKEN_CHARSET,
    PARSE_TOKEN_CHARSET_STR,
    PARSE_RPAREN,       
    PARSE_LPAREN,
    PARSE_ALTERNATION,
    PARSE_CONCAT,
    PARSE_KLEEN,
    PARSE_QUESTION,
    PARSE_PLUS,
    PARSE_EMPTY_STRING,
};

typedef enum parse_tolken_e parse_tolken_t;

uint32_t
parse_charset(
    const unsigned char * const restrict str,
    uint32_t const dparam, const char ** deriv
);

uint32_t
parse_regex(
    const unsigned char * const restrict str,
    parse_tolken_t * const restrict token
);

#endif /* CHARSET_PARSE_H */
