#ifndef IR_H_
#define IR_H_

#include <stdint.h>

typedef enum expr_type_t {
    SYMBOL,
    LIST,
    PROC_SYMBOL,
    NUMBER,
    CONS,
    DEFUN,
    DEFINE
} expr_type;

typedef struct expr_t {
    expr_type type;
    uint64_t data;
    struct expr_t *next;
} expr;

expr *read_from_tokens(char **tokens);

#endif // IR_H_
