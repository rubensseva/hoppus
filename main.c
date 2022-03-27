#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>

#include "tokenize.h"
#include "ir.h"
#include "eval.h"

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

    char *buf = malloc(1024);
    int bytes_read = read(fd, buf, 1024);
    if (bytes_read == -1) {
        perror("read");
        return 1;
    }

    expr *res = eval(read_from_tokens(tokenize(buf)));
    // printf("result type: %d\n", res->type);
    printf("%lu\n", res->data);

}
