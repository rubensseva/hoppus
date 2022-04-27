#ifndef PARSER_H_
#define PARSER_H_

#include <expr.h>

#include <stdint.h>


int parse_tokens(char **tokens, int fd, expr **res);
expr *continually_read_from_tokens(char **tokens);


#endif // PARSER_H_
