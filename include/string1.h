#ifndef STRTOK1_H_
#define STRTOK1_H_

#include <stdint.h>

char* strtok1(char *str, const char* delim);
char* trim1(char *str, char delim);
uint32_t strlen1(char *str);
uint32_t strcmp1(char *str1, char *str2);
uintptr_t strcpy1(char *str, char *buf);
int atoi1(char* str);

#endif // STRTOK1_H_
