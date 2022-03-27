#include <ctype.h>
#include "utility.h"


/* Taken from https://www.codegrepper.com/code-examples/c/c+isnumber */
int is_number(char *s) {
    for (int i = 0; s[i]!= '\0'; i++)
    {
        if (isdigit(s[i]) == 0)
              return 0;
    }
    return 1;
}
