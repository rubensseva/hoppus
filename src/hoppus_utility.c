#include <string1.h>
#include <hoppus_utility.h>
#include <hoppus_constants.h>

#include <stdint.h>
#include <link.h>

__USER_TEXT int isdigit(char c) {
    return c >= '0' && c <= '9';
}

/* Taken from https://www.codegrepper.com/code-examples/c/c+isnumber */
__USER_TEXT int is_number(char *s) {
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

__USER_TEXT int is_boolean(char *s) {
    if (strcmp1(s, BOOL_STR_T) == 0 || strcmp1(s, BOOL_STR_F) == 0) {
        return 1;
    }
    return 0;
}

__USER_TEXT int is_string(char *s) {
    unsigned int size = strlen1(s);
    if (size < 2)
        return 0;
    if (s[0] != '"')
        return 0;
    if (s[size - 1] != '"')
        return 0;
    return 1;
}

__USER_TEXT int is_nil(char *s) {
    if (strcmp1(s, NIL_STR) == 0) {
        return 1;
    }
    return 0;
}

__USER_TEXT int insert_char_in_str(char *str, int i, char c) {
    int str_size = strlen1(str);
    if (i < 0 || i == str_size)
        return -1;

    for (int j = str_size; j > i; j--)
        str[j] = str[j - 1];

    str[str_size + 1] = '\0';
    str[i] = c;
    return 0;
}
