#include <ctype.h>
#include <string.h>
#include "utility.h"


/* Taken from https://www.codegrepper.com/code-examples/c/c+isnumber */
int is_number(char *s) {
    for (int i = 0; s[i]!= '\0'; i++) {
        if (i == 0 && s[i] == '-') {
            if (s[i + 1] == '\0' || !isdigit(s[i + 1])) {
                return 0;
            }
        } else if (isdigit(s[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

int is_string(char *s) {
    unsigned int size = strlen(s);
    if (size < 2)
        return 0;
    if (s[0] != '"')
        return 0;
    if (s[size - 1] != '"')
        return 0;
    return 1;
}

int insert_char_in_str(char *str, int i, char c) {
    int str_size = strlen(str);
    if (i < 0 || i == str_size)
        return -1;

    for (int j = str_size; j > i; j--)
        str[j] = str[j - 1];

    str[str_size + 1] = '\0';
    str[i] = c;
    return 0;
}
