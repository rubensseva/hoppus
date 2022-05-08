#include <hoppus_constants.h>
#include <hoppus_link.h>

__USER_DATA char WHITESPACE[] = " ";
__USER_DATA char OPENING_PAREN[] = "(";
__USER_DATA char CLOSING_PAREN[] = ")";
__USER_DATA char DOUBLEQUOTE[] = " ";

__USER_DATA char QUOTE_STR[] = "quote";
__USER_DATA char QUOTE_SHORT_STR[] = "'";
__USER_DATA char QUASIQUOTE_STR[] = "quasiquote";
__USER_DATA char QUASIQUOTE_SHORT_STR[] = "`";
__USER_DATA char COMMA_STR[] = "comma";
__USER_DATA char COMMA_SHORT_STR[] = ",";
__USER_DATA char COMMA_AT_STR[] = "comma-at";
__USER_DATA char COMMA_AT_SHORT_STR[] = ",@";

__USER_DATA char DEFUN_STR[] = "defun";
__USER_DATA char DEFINE_STR[] = "define";
__USER_DATA char ADD_STR[] = "+";
__USER_DATA char SUB_STR[] = "-";
__USER_DATA char MULT_STR[] = "*";
__USER_DATA char DIV_STR[] = "/";
__USER_DATA char CONS_STR[] = "cons";
__USER_DATA char CAR_STR[] = "car";
__USER_DATA char CDR_STR[] = "cdr";
__USER_DATA char PROGN_STR[] = "progn";
__USER_DATA char COND_STR[] = "cond";
__USER_DATA char PRINT_STR[] = "print";
__USER_DATA char EQ_STR[] = "eq";
__USER_DATA char GT_STR[] = "gt";
__USER_DATA char LT_STR[] = "lt";
__USER_DATA char AND_STR[] = "and";
__USER_DATA char OR_STR[] = "or";
// symbol *quote = symbol_builtin_create(QUOTE_STR, bi_quote, 1);
__USER_DATA char DEFMACRO_STR[] = "defmacro";
__USER_DATA char MACROEXPAND_STR[] = "macroexpand";
/* __USER_DATA char QUASIQUOTE_STR[] = "quasiquote"; */
/* __USER_DATA char COMMA_STR[] = "comma"; */
/* __USER_DATA char COMMA_AT_STR[] = "comma-at"; */

__USER_DATA char BOOL_STR_T[] = "true";
__USER_DATA char BOOL_STR_F[] = "false";

__USER_DATA char NIL_STR[] = "nil";

__USER_DATA char REST_ARGUMENTS_STR[] = "&rest";
