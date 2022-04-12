#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include "utility.h"
#include "constants.h"


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

int is_boolean(char *s) {
    if (strcmp(s, BOOL_STR_T) == 0 || strcmp(s, BOOL_STR_F) == 0) {
        return 1;
    }
    return 0;
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

int is_nil(char *s) {
    if (strcmp(s, NIL_STR) == 0) {
        return 1;
    }
    return 0;
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

unsigned int *word_align_up(unsigned int *ptr) {
    if ((uint64_t) ptr % 8 != 0)
        ptr += 8 - (uint64_t) ptr % 8;
    return ptr;
}
unsigned int *word_align_down(unsigned int *ptr) {
    if ((uint64_t) ptr % 8 != 0)
        ptr -= (uint64_t) ptr % 8;
    return ptr;
}
