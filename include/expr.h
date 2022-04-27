#ifndef EXPR_H_
#define EXPR_H_

#include <stdint.h>

/* This is a little bit hacky, but it works.
   As long as we dont define values for the enum below, it will grow from 0 and increase
   by one for each element. We need the EXPR_TYPE_ENUM_SIZE variable to check if
   arguments to type functions is out of bounds, which then works as long as we
   don't define values for the enum.

   This is used when getting a string represention of the enum, to make sure we dont
   go out of bounds.

   If you want to add an enum type, follow these steps:

   1. Add the type to the enum below, DONT give it an enum value
   2. Increment EXPR_TYPE_ENUM_SIZE
   3. Add a human readable string to expr_type_string_map in parser.c, make sure the index in that array
      matches the implicit value of the enum type you just added (maybe make sure the other ones are
      also still correct)
*/
#define EXPR_TYPE_ENUM_SIZE 5
typedef enum expr_type_t {
    SYMBOL,
    NUMBER,
    CHAR,
    CONS,
    BOOLEAN
} expr_type;

typedef struct expr_t expr;
struct expr_t {
    expr *car;  /* Data field if other type than cons cell */
    expr *cdr;  /* LSB designates if it is cons cell or anything else. */
};

char *type_str(expr_type tp);

expr *car(expr *e);
void set_car(expr *e, expr *new_car);
expr *cdr(expr *e);
void set_cdr(expr *e, expr* new_cdr);
uint32_t data(expr *e);
void set_data(expr *e, uint32_t data);
expr_type type(expr *e);
void set_type(expr *e, expr_type type);

/* type of the car */
expr_type tar(expr *e);
/* data of the car */
uint32_t dar(expr *e);

expr *nth(unsigned int i, expr *e);

expr *expr_new_val(expr_type type, uint32_t data);
expr *expr_new_cons(expr* car, expr *cdr);
int expr_copy(expr *e, expr **out);

int expr_is_true(expr *e);
int expr_is_equal(expr *e1, expr *e2);
int expr_gt_lt(expr *e1, expr *e2, int is_gt);

int expr_print(expr *e);

#endif // EXPR_H_
