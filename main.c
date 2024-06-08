#include <stdio.h>
#include <string.h>

#include "charset_parse.h"


int
main()
{
    char str[256];
    int r, n;
    r = 0;
    const char * d;
    while (fgets(str, 256, stdin))
    {
        for (n = 0; str[n] && str[n] != '\n'; n++);
        str[n] = 0;
        n = parse_charset(str, 'a', &d);
        printf(
            "%s : %s -> ",
            str,
            n == strlen(str)? "SUCCESS" : "FAIL"
        );
        printf(
            "%s\n",
            d == NULL? "NULL" : "EMPTY STRING"
        );
        if (n) r = n;
    }
    return r;
}

