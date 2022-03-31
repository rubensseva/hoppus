#ifndef TOKENIZE_H_
#define TOKENIZE_H_

typedef char* token_t;

char **read_tokens_from_file(int fd);
token_t *tokens_init();
void tokens_free(char **tokens);
int tokens_add(char **tokens, char **new_tokens);
char *tokens_pop(char **tokens, int fd);
char *tokens_peek(char **tokens, int fd);

char** tokenize(char *src_code);

#endif // TOKENIZE_H_
