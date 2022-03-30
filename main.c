#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>

#include "tokenize.h"
#include "ir.h"
#include "eval.h"
#include "memory.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("USAGE: %s <filename>\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    char *buf = my_malloc(1024);
    int bytes_read = read(fd, buf, 1024);
    if (bytes_read == -1) {
        perror("read");
        return 1;
    }
    buf[bytes_read] = '\0';

    char **tokens = tokenize(buf);
    expr *root = continually_read_from_tokens(tokens);

    free_tokens(tokens);

    expr *res = eval(root);
    printf("%lu\n", res->data);

    my_free(buf);
    free_tree(root);
    free_tree(res);
}
