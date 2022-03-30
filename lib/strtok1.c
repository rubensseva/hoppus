#include "lib/strtok1.h"

#define NULL (void *)0x0

/**
   Taken from https://fengl.org/2013/01/01/implement-strtok-in-c-2/
*/
char* strtok1(char *str, const char* delim) {
    static char* _buffer;
    if(str != NULL) _buffer = str;
    if(_buffer[0] == '\0') return NULL;

    char *ret = _buffer, *b;
    const char *d;

    for(b = _buffer; *b !='\0'; b++) {
        for(d = delim; *d != '\0'; d++) {
            if(*b == *d) {
                *b = '\0';
                _buffer = b+1;

                // skip the beginning delimiters
                if(b == ret) {
                    ret++;
                    continue;
                }
                return ret;
            }
        }
    }

    return ret;
}
