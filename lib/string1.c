#include "lib/string1.h"
#include "utility.h"

#include <string.h>

#define NULL (void *)0x0

/**
   Trims delim from start and end of str.
   Modifies the str. The returned string is just the str argument, possibly
   another position in the str argument.
*/
char* trim1(char *str, char delim) {
    unsigned int size = strlen(str);
    int new_start = -1;
    for (int i = 0; i < size; i++) {
        if (str[i] != delim) {
            new_start = i;
            break;
        }
    }

    str = &str[new_start];
    size = strlen(str);

    int new_end = -1;
    for (int i = size - 1; i > 0; i--) {
        if (str[i] != delim) {
            break;
        }
        str[i] = '\0';
    }
    return str;
}

int char_in_str(char c, char *s) {
    unsigned int size = strlen(s);
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
char* strtok1(char *str, const char* delim) {
    static char* _buffer;
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
