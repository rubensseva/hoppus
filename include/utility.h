#ifndef UTILITY_H_
#define UTILITY_H_

int is_number(char *s);
int is_boolean(char *s);
int is_string(char *s);
int is_nil(char *s);
int insert_char_in_str(char *str, int i, char c);
unsigned int *word_align_up(unsigned int *ptr);
unsigned int *word_align_down(unsigned int*ptr);

#endif // UTILITY_H_
