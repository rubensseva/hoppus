#include <ctype.h>
#include <string.h>
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

int insert_char_in_str(char *str, int i, char c) {
    int str_size = strlen(str);
    if (i < 0 || i == str_size) {
        return -1;
    }
    for (int j = str_size; j > i; j--) {
        str[j] = str[j - 1];
    }
    str[str_size + 1] = '\0';
    str[i] = c;
    return 0;
}
