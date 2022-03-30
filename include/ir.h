#ifndef IR_H_
#define IR_H_

#include <stdint.h>

typedef enum expr_type_t {
    ROOT,
    SEQUENCE_HOLDER,
    SYMBOL,
    PROC_SYMBOL,
    NUMBER,
    CONS,
} expr_type;

typedef struct expr_t {
    expr_type type;
    uint64_t data;
    struct expr_t *car;
    struct expr_t *cdr;
} expr;

expr *read_from_tokens(char **tokens);
expr *continually_read_from_tokens(char **tokens);

expr *expr_new(expr_type type, uint64_t data, expr* car, expr *cdr);
expr *expr_cons(expr* car, expr *cdr);

#endif // IR_H_
