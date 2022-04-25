#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <string.h>

#include "gc.h"
#include "tokenize.h"
#include "parser.h"
#include "eval.h"
#include "memory.h"
#include "symbol.h"
#include "builtins.h"
#include "config.h"
#include "constants.h"
#include "lisp_lib.h"

void create_builtins() {
    symbol *defun = symbol_builtin_create("defun", bi_defun, 1);
    symbol *define = symbol_builtin_create("define", bi_define, 1);
    symbol *add = symbol_builtin_create("+", bi_add, 0);
    symbol *sub = symbol_builtin_create("-", bi_sub, 0);
    symbol *mult = symbol_builtin_create("*", bi_mult, 0);
    symbol *div = symbol_builtin_create("/", bi_div, 0);
    symbol *cons = symbol_builtin_create("cons", bi_cons, 0);
    symbol *car = symbol_builtin_create("car", bi_car, 0);
    symbol *cdr = symbol_builtin_create("cdr", bi_cdr, 0);
    symbol *progn = symbol_builtin_create("progn", bi_progn, 0);
    symbol *cond = symbol_builtin_create("cond", bi_cond, 1);
    symbol *_print = symbol_builtin_create("print", bi_print, 0);
    symbol *equal = symbol_builtin_create("eq", bi_equal, 0);
    symbol *_gt = symbol_builtin_create("gt", bi_gt, 0);
    symbol *_lt = symbol_builtin_create("lt", bi_lt, 0);
    symbol *_and = symbol_builtin_create("and", bi_and, 1);
    symbol *_or = symbol_builtin_create("or", bi_or, 1);
    symbol *quote = symbol_builtin_create(QUOTE_STR, bi_quote, 1);
    symbol *defmacro = symbol_builtin_create("defmacro", bi_defmacro, 1);
    symbol *macroexpand = symbol_builtin_create("macroexpand", bi_macroexpand, 1);
    symbol *quasiquote = symbol_builtin_create("quasiquote", bi_quasiquote, 1);
    symbol *comma = symbol_builtin_create("comma", bi_comma, 1);
    symbol *comma_at = symbol_builtin_create("comma-at", bi_comma_at, 1);

    symbol_add(defun);
    symbol_add(define);
    symbol_add(add);
    symbol_add(sub);
    symbol_add(mult);
    symbol_add(div);
    symbol_add(cons);
    symbol_add(car);
    symbol_add(cdr);
    symbol_add(progn);
    symbol_add(cond);
    symbol_add(_print);
    symbol_add(equal);
    symbol_add(_gt);
    symbol_add(_lt);
    symbol_add(_and);
    symbol_add(_or);
    symbol_add(quote);
    symbol_add(defmacro);
    symbol_add(macroexpand);
    symbol_add(quasiquote);
    symbol_add(comma);
    symbol_add(comma_at);
}

void parser_error(int error) {
    switch (error) {
        case GENERIC_ERROR:
            printf("ERROR: MAIN: PARSER: generic error, no more info\n");
        case EOF_WHILE_READING_EXPR_ERROR_CODE:
            printf("ERROR: MAIN: PARSER: got EOF while parsing an unfinished LISP list. Unmatched opening parentheses?\n");
        default:
            printf("ERROR: MAIN: PARSER: unknown error: %d\n", error);
    }
}
void eval_error(int error) {
    switch (error) {
        case GENERIC_ERROR:
            printf("ERROR: MAIN: EVAL: generic error\n");
            break;
        case UNBOUND_SYMBOL_NAME_ERROR:
            printf("ERROR: MAIN: EVAL: unbound symbol name\n");
            break;
        case NUMBER_OF_ARGUMENTS_ERROR:
            printf("ERROR: MAIN: EVAL: wrong number of arguments\n");
            break;
        case TYPE_ERROR:
            printf("ERROR: MAIN: EVAL: type error\n");
            break;
        case INVALID_FORM_ERROR:
            printf("ERROR: MAIN: EVAL: invalid form\n");
            break;
        default:
            printf("ERROR: MAIN: EVAL: unknown error: %d\n", error);
    }
}

int load_standard_library() {
    int ret_code;
    /* Load standard library */
    for (int i = 0; i < (sizeof(lib_strs) / sizeof(char *)); i++) {
        token_t *tokens = tokens_init();
        char *copy = my_malloc(EXPR_STR_MAX_LEN);
        strcpy(copy, (char *)lib_strs[i]);
        ret_code = tokenize(copy, tokens);
        if (ret_code < 0) {
            printf("ERROR: MAIN: tokenizing standard library string %s\n", (char *)lib_strs[0]);
            return ret_code;
        }

        expr *parsed;
        ret_code = parse_tokens(tokens, 0, &parsed);
        if (ret_code < 0) {
            printf("ERROR: MAIN: parsing standard library tokens %s\n", (char *) lib_strs[0]);
            parser_error(ret_code);
            return ret_code;
        }

        expr *evald;
        ret_code = eval(parsed, &evald);
        if (ret_code < 0) {
            printf("ERROR: MAIN: evaluating standard library forms: %s\n", (char *) lib_strs[0]);
            eval_error(ret_code);
            return ret_code;
        }
    }
    return 0;
}

int REPL_loop(int fd) {
    int ret_code;
    token_t *tokens = tokens_init();
    expr *parsed = NULL, *evald = NULL;
    while (1) {
        if (fd == 1) {
            printf("$ ");
            fflush(stdout);
        }

        ret_code = parse_tokens(tokens, fd, &parsed);
        if (ret_code < 0) {
            parser_error(ret_code);
            return ret_code;
        }
        if (ret_code == EOF_CODE) {
            printf("INFO: MAIN: EOF\n");
            printf("INFO: MAIN: return value: ");
            expr_print(evald);
            return 0;
        }

        ret_code = eval(parsed, &evald);
        if (ret_code < 0) {
            eval_error(ret_code);
            return ret_code;
        } else {
            if (fd == 1) {
                expr_print(evald);
            }
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    int ret_code;
    printf("INFO: MAIN: welcome to ukernel lisp!\n");
    if (argc > 2) {
        printf("USAGE: %s <filename>\n", argv[0]);
        return -1;
    }
    gc_init();
    printf("INFO: MAIN: gc initialized\n");

    create_builtins();
    printf("INFO: MAIN: builtins created\n");

    ret_code = load_standard_library();
    if (ret_code < 0) {
        printf("ERROR: MAIN: loading standard library: %d\n", ret_code);
        return -1;
    }
    printf("INFO: MAIN: standard library loaded\n");

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

    printf("INFO: MAIN: starting REPL loop\n");
    ret_code = REPL_loop(fd);
    if (ret_code < 0) {
        printf("ERROR: MAIN: %d\n", ret_code);
        return -1;
    }

    /* if (fd != 1) { */
    /*     close(fd); */
    /*     printf("INFO: MAIN: starting REPL loop after file read\n"); */
    /*     ret_code = REPL_loop(1); */
    /*     if (ret_code < 0) { */
    /*         printf("ERROR: MAIN: %d\n", ret_code); */
    /*         return -1; */
    /*     } */
    /* } */


    printf("INFO: MAIN: GC: num mallocs: %lu\n", gc_stats_get_num_malloc());
    printf("INFO: MAIN: GC: total allocated: %lu\n", gc_stats_get_allocated_total());
    printf("INFO: MAIN: exiting with code: %d\n", ret_code);
    printf("INFO: MAIN: bye...\n");
    return ret_code;
}
