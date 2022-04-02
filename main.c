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

    symbol *defun = symbol_builtin_create("defun", bi_defun, 1);
    symbol *define = symbol_builtin_create("define", bi_define, 1);
    symbol *add = symbol_builtin_create("+", bi_add, 0);
    symbol *sub = symbol_builtin_create("-", bi_sub, 0);
    symbol *cons = symbol_builtin_create("cons", bi_cons, 0);
    symbol *car = symbol_builtin_create("car", bi_car, 0);
    symbol *cdr = symbol_builtin_create("cdr", bi_cdr, 0);
    symbol *progn = symbol_builtin_create("progn", bi_progn, 0);
    symbol *_if = symbol_builtin_create("if", bi_if, 1);
    symbol *_print = symbol_builtin_create("print", bi_print, 0);
    symbol *equal = symbol_builtin_create("eq", bi_equal, 0);
    symbol *_gt = symbol_builtin_create("gt", bi_gt, 0);
    symbol *_lt = symbol_builtin_create("lt", bi_lt, 0);

    symbol_add(defun);
    symbol_add(define);
    symbol_add(add);
    symbol_add(sub);
    symbol_add(cons);
    symbol_add(car);
    symbol_add(cdr);
    symbol_add(progn);
    symbol_add(_if);
    symbol_add(_print);
    symbol_add(equal);
    symbol_add(_gt);
    symbol_add(_lt);

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

    token_t *tokens = tokens_init();
    while (1) {
        if (fd == 1)
            printf("$ ");
        fflush(stdout);

        expr *curr, *evald;
        int res = parse_tokens(tokens, fd, &curr);
        if (res < 0) {
            printf("MAIN: ERROR: Parser: %d\n", res);
            return -1;
        }
        if (res == EOF_CODE) {
            printf("MAIN: Reached EOF\n");
            return -1;
        }
        evald = eval(curr);
        if (evald == NULL) {
            printf("MAIN: ERROR: Eval\n");
        } else {
            printf("Return: \n");
            print_expr(evald);
        }
    }

    /* TODO: Free remaining memory allocations */
}
