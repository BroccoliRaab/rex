#ifndef CHARSET_PARSE_H
#define CHARSET_PARSE_H

#include <stdint.h>

uint32_t
parse_charset(
    const unsigned char * const restrict str,
    uint32_t const dparam, const char ** deriv
);

#endif /* CHARSET_PARSE_H */
