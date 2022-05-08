#include <string1.h>
#include <gc.h>
#include <tokenize.h>
#include <parser.h>
#include <eval.h>
#include <symbol.h>
#include <builtins.h>
#include <hoppus_memory.h>
#include <hoppus_config.h>
#include <hoppus_constants.h>
#include <hoppus_stdio.h>
#include <lisp_lib.h>
#include <symbol.h>
#include <hoppus_link.h>


__USER_TEXT void create_builtins() {
    symbol *defun = symbol_builtin_create(DEFUN_STR, bi_defun, 1);
    symbol *define = symbol_builtin_create(DEFINE_STR, bi_define, 1);
    symbol *add = symbol_builtin_create(ADD_STR, bi_add, 0);
    symbol *sub = symbol_builtin_create(SUB_STR, bi_sub, 0);
    symbol *mult = symbol_builtin_create(MULT_STR, bi_mult, 0);
    symbol *div = symbol_builtin_create(DIV_STR, bi_div, 0);
    symbol *cons = symbol_builtin_create(CONS_STR, bi_cons, 0);
    symbol *car = symbol_builtin_create(CAR_STR, bi_car, 0);
    symbol *cdr = symbol_builtin_create(CDR_STR, bi_cdr, 0);
    symbol *progn = symbol_builtin_create(PROGN_STR, bi_progn, 0);
    symbol *cond = symbol_builtin_create(COND_STR, bi_cond, 1);
    symbol *_print = symbol_builtin_create(PRINT_STR, bi_print, 0);
    symbol *equal = symbol_builtin_create(EQ_STR, bi_equal, 0);
    symbol *_gt = symbol_builtin_create(GT_STR, bi_gt, 0);
    symbol *_lt = symbol_builtin_create(LT_STR, bi_lt, 0);
    symbol *_and = symbol_builtin_create(AND_STR, bi_and, 1);
    symbol *_or = symbol_builtin_create(OR_STR, bi_or, 1);
    symbol *quote = symbol_builtin_create(QUOTE_STR, bi_quote, 1);
    symbol *defmacro = symbol_builtin_create(DEFMACRO_STR, bi_defmacro, 1);
    symbol *macroexpand = symbol_builtin_create(MACROEXPAND_STR, bi_macroexpand, 1);
    symbol *quasiquote = symbol_builtin_create(QUASIQUOTE_STR, bi_quasiquote, 1);
    symbol *comma = symbol_builtin_create(COMMA_STR, bi_comma, 1);
    symbol *comma_at = symbol_builtin_create(COMMA_AT_STR, bi_comma_at, 1);

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

__USER_TEXT void parser_error(int error) {
    switch (error) {
        case GENERIC_ERROR:
            hoppus_puts("ERROR: MAIN: PARSER: generic error, no more info\n");
        case EOF_WHILE_READING_EXPR_ERROR_CODE:
            hoppus_puts("ERROR: MAIN: PARSER: got EOF while parsing an unfinished LISP list. Unmatched opening parentheses?\n");
        default:
            hoppus_printf("ERROR: MAIN: PARSER: unknown error: %d\n", error);
    }
}
__USER_TEXT void eval_error(int error) {
    switch (error) {
        case GENERIC_ERROR:
            hoppus_puts("ERROR: MAIN: EVAL: generic error\n");
            break;
        case UNBOUND_SYMBOL_NAME_ERROR:
            hoppus_puts("ERROR: MAIN: EVAL: unbound symbol name\n");
            break;
        case NUMBER_OF_ARGUMENTS_ERROR:
            hoppus_puts("ERROR: MAIN: EVAL: wrong number of arguments\n");
            break;
        case TYPE_ERROR:
            hoppus_puts("ERROR: MAIN: EVAL: type error\n");
            break;
        case INVALID_FORM_ERROR:
            hoppus_puts("ERROR: MAIN: EVAL: invalid form\n");
            break;
        default:
            hoppus_printf("ERROR: MAIN: EVAL: unknown error: %d\n", error);
    }
}

__USER_TEXT int load_standard_library() {
    int ret_code;
    /* Load standard library */
    for (int i = 0; i < (sizeof(lib_strs) / sizeof(char *)); i++) {
        token_t *tokens = tokens_init();
        char *copy = my_malloc(EXPR_STR_MAX_LEN);
        strcpy1(copy, (char *)lib_strs[i]);
        ret_code = tokenize(copy, tokens);
        if (ret_code < 0) {
            hoppus_printf("ERROR: MAIN: tokenizing standard library string %s\n", (char *)lib_strs[0]);
            return ret_code;
        }

        expr *parsed;
        ret_code = parse_tokens(tokens, 0, &parsed);
        if (ret_code < 0) {
            hoppus_printf("ERROR: MAIN: parsing standard library tokens %s\n", (char *) lib_strs[0]);
            parser_error(ret_code);
            return ret_code;
        }
        expr_print(parsed);

        expr *evald;
        ret_code = eval(parsed, &evald);
        if (ret_code < 0) {
            hoppus_printf("ERROR: MAIN: evaluating standard library forms: %s\n", (char *) lib_strs[0]);
            eval_error(ret_code);
            return ret_code;
        }
        expr_print(evald);
    }
    return 0;
}

__USER_TEXT int REPL_loop(int fd) {
    int ret_code;
    token_t *tokens = tokens_init();
    expr *parsed = NULL, *evald = NULL;
    while (1) {
        hoppus_puts("$ ");

        ret_code = parse_tokens(tokens, fd, &parsed);
        if (ret_code < 0) {
            parser_error(ret_code);
            return ret_code;
        }
        if (ret_code == EOF_CODE) {
            hoppus_puts("INFO: MAIN: EOF\n");
            hoppus_puts("INFO: MAIN: return value: ");
            expr_print(evald);
            return 0;
        }

        ret_code = eval(parsed, &evald);
        if (ret_code < 0) {
            eval_error(ret_code);
            return ret_code;
        } else {
            expr_print(evald);
        }
    }
    return 0;
}



__USER_TEXT int clisp_main() {
    int ret_code;

    hoppus_puts("INFO: MAIN: welcome to Hoppus!\n");
    gc_init();
    hoppus_puts("INFO: MAIN: gc initialized\n");

    create_builtins();
    hoppus_puts("INFO: MAIN: builtins created\n");

    ret_code = load_standard_library();
    if (ret_code < 0) {
        hoppus_printf("ERROR: MAIN: loading standard library: %d\n", ret_code);
        return -1;
    }
    hoppus_puts("INFO: MAIN: standard library loaded\n");

    int fd = 0;

    hoppus_puts("INFO: MAIN: starting REPL loop\n");

    ret_code = REPL_loop(fd);
    if (ret_code < 0) {
        hoppus_printf("ERROR: MAIN: %d\n", ret_code);
        return -1;
    }

    /* if (fd != 1) { */
    /*     close(fd); */
    /*     hoppus_puts("INFO: MAIN: starting REPL loop after file read\n"); */
    /*     ret_code = REPL_loop(1); */
    /*     if (ret_code < 0) { */
    /*         hoppus_printf("ERROR: MAIN: %d\n", ret_code); */
    /*         return -1; */
    /*     } */
    /* } */


    hoppus_printf("INFO: MAIN: GC: num mallocs: %lu\n", gc_stats_get_num_malloc());
    hoppus_printf("INFO: MAIN: exiting with code: %d\n", ret_code);
    hoppus_puts("INFO: MAIN: bye...\n");
    return ret_code;
}

#ifdef HOPPUS_PLATFORM
#if HOPPUS_PLATFORM == HOPPUS_X86
int main() {
    return clisp_main();
}
#endif
#endif
