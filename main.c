#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <string.h>

#include "tokenize.h"
#include "parser.h"
#include "eval.h"
#include "memory.h"
#include "symbol.h"
#include "builtins.h"
#include "config.h"

int main(int argc, char **argv) {
    if (argc > 2) {
        printf("USAGE: %s <filename>\n", argv[0]);
        return -1;
    }

    symbol *add = symbol_builtin_create("+", bi_add);
    symbol *sub = symbol_builtin_create("-", bi_sub);
    symbol *cons = symbol_builtin_create("cons", bi_cons);
    symbol *car = symbol_builtin_create("car", bi_car);
    symbol *cdr = symbol_builtin_create("cdr", bi_cdr);

    symbol_add(add);
    symbol_add(sub);
    symbol_add(cons);
    symbol_add(car);
    symbol_add(cdr);

    int fd;
    if (argc == 2) {
        fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror("open");
            return -1;
        }
    } else {
        fd = 1;
    }

    /* char buf[EXPR_STR_SIZE]; */
    while (1) {
        if (fd == 1)
            printf("$ ");
        fflush(stdout);
        /* int bytes_read = read(fd, buf, EXPR_STR_SIZE); */
        /* if (bytes_read == -1) { */
        /*     perror("read"); */
        /*     return -1; */
        /* } */
        /* buf[bytes_read] = '\0'; */

        token_t *tokens = tokens_init();
        expr *curr, *evald;
        curr = parse_tokens(tokens, fd);
        if (curr == NULL) {
            printf("MAIN: ERROR: Parser\n");
            return -1;
        }
        evald = eval(curr);
        if (evald == NULL) {
            printf("MAIN: ERROR: Eval\n");
        } else if (evald->data) {
            printf("%lu\n", (uint64_t) evald->data);
        }
    }

    /* TODO: Free remaining memory allocations */
}
