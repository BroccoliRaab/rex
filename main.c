#include <stdio.h>

#include "charset_parse.h"

int
main()
{
    char str[256];
    int r, n;
    r = 0;
    while (fgets(str, 256, stdin))
    {
        for (n = 0; str[n] && str[n] != '\n'; n++);
        str[n] = 0;
        n = parse_charset(str, 0, NULL);
        printf(
            "%s : %s\n",
            str,
            n ? "SUCCESS" : "FAIL"
        );
        if (n) r = n;
    }
    return r;
}

