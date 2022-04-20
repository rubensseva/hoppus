#ifndef TOKENIZE_H_
#define TOKENIZE_H_

typedef char* token_t;

int read_tokens_from_file(int fd, token_t *dest);
token_t *tokens_init();
int tokens_add(char **tokens, char **new_tokens);
int tokens_pop(token_t *tokens, int fd, token_t *dest);
int tokens_peek(char **tokens, int fd, token_t *dest);

int tokenize(char *src_code, token_t *dest);

#endif // TOKENIZE_H_
