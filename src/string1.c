#include <string1.h>
#include <hoppus_utility.h>

#include <link.h>
#include <types.h>
#include <user_stdio.h>


// #define NULL (void *)0x0

/**
   Trims delim from start and end of str.
   Modifies the str. The returned string is just the str argument, possibly
   another position in the str argument.
*/
__USER_TEXT char* trim1(char *str, char delim) {
    unsigned int size = strlen1(str);
    int new_start = -1;
    for (int i = 0; i < size; i++) {
        if (str[i] != delim) {
            new_start = i;
            break;
        }
    }

    str = &str[new_start];
    size = strlen1(str);

    int new_end = -1;
    for (int i = size - 1; i > 0; i--) {
        if (str[i] != delim) {
            break;
        }
        str[i] = '\0';
    }
    return str;
}

__USER_TEXT int char_in_str(char c, char *s) {
    unsigned int size = strlen1(s);
    for (int i = 0; i < size; i++) {
        if (c == s[i]) {
            return 1;
        }
    }
    return 0;
}

/**
   Inspired from https://fengl.org/2013/01/01/implement-strtok-in-c-2/

   str must be trimmed for delim at start and end
*/
__USER_TEXT char* strtok1(char *str, const char* delim) {
    static __USER_DATA char* _buffer;
    if(str != NULL) _buffer = str;
    if(_buffer[0] == '\0') return NULL;

    char *ret = _buffer, *b;
    int in_str = 0, skipping_delims = 0;
    for(b = _buffer; *b !='\0'; b++) {
        /* If we are currently skipping delims, and we find a char that is
           not a delim, then we are done skipping delims */
        if (skipping_delims && !char_in_str(*b, (char *) delim)) {
            _buffer = b;
            return ret;
        }

        /* Check if we are in a string, in that case we need to
           ignore it */
        if (*b == '"')
            in_str = !in_str;
        if (in_str)
            continue;

        /* If we find a delim we set that char to '\0', and we set delim_found
           to 1 to signal that we should start skipping delims (in case several
           delims are in sequence) */
        if (char_in_str(*b, (char *)delim)) {
            *b = '\0';
            skipping_delims = 1;
        }
    }
    _buffer = b;
    return ret;
}


__USER_TEXT uint32_t strlen1(char *str) {
    if (str == NULL) {
        user_puts("STR is NULL\n");
    }
    for (int i = 0;;i++) {
        if (str[i] == '\0')
            return i;
    }
}

__USER_TEXT uint32_t strcmp1(char *str1, char *str2) {
    if (strlen1(str1) != strlen1(str2)) {
        return 1;
    }
    for (int i = 0; i < strlen1(str1); i++) {
        if (str1[i] != str2[i]) {
            return 1;
        }
    }
    return 0;
}

__USER_TEXT uint32_t strcpy1(char *dest, char *src) {
    for (int i = 0; i < strlen1(src); i++) {
        dest[i] = src[i];
    }
    dest[strlen1(src)] = '\0';
    return 0;
}


/* From https://www.geeksforgeeks.org/write-your-own-atoi/ */
__USER_TEXT int atoi1(char* str)
{
    int res = 0;
    int sign = 1;
    int i = 0;
    if (str[0] == '-') {
        sign = -1;
        i++;
    }
    for (; str[i] != '\0'; i++)
        res = res * 10 + str[i] - '0';
    return sign * res;
}
