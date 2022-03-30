#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>

#include "tokenize.h"
#include "ir.h"
#include "eval.h"
#include "memory.h"
#include "symbol.h"
#include "builtins.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("USAGE: %s <filename>\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    char *buf = my_malloc(1024);
    int bytes_read = read(fd, buf, 1024);
    if (bytes_read == -1) {
        perror("read");
        return -1;
    }
    buf[bytes_read] = '\0';

    char **tokens = tokenize(buf);

    // expr *root = continually_read_from_tokens(tokens);

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

    // expr *res = eval(root);
    expr *curr, *evald;
    while (tokens[0]) {
        curr = read_from_tokens(tokens);
        evald = eval(curr);
        if (evald->data) {
            printf("%lu\n", (uint64_t) evald->data);
        }
    }

    free_tokens(tokens);
    // free_tree(curr);

    printf("%lu\n", evald->data);

    my_free(buf);
    // free_tree(root);
    free_tree(evald);
}
