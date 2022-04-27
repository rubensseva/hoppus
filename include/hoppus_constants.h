#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <link.h>

/*
  Constants that should be generally available, but not
  be part of the config go here.
*/

extern char WHITESPACE[];
extern char OPENING_PAREN[];
extern char CLOSING_PAREN[];
extern char DOUBLEQUOTE[];

extern char QUOTE_STR[];
extern char QUOTE_SHORT_STR[];
extern char QUASIQUOTE_STR[];
extern char QUASIQUOTE_SHORT_STR[];
extern char COMMA_STR[];
extern char COMMA_SHORT_STR[];
extern char COMMA_AT_STR[];
extern char COMMA_AT_SHORT_STR[];

extern char DEFUN_STR[];
extern char DEFINE_STR[];
extern char ADD_STR[];
extern char SUB_STR[];
extern char MULT_STR[];
extern char DIV_STR[];
extern char CONS_STR[];
extern char CAR_STR[];
extern char CDR_STR[];
extern char PROGN_STR[];
extern char COND_STR[];
extern char PRINT_STR[];
extern char EQ_STR[];
extern char GT_STR[];
extern char LT_STR[];
extern char AND_STR[];
extern char OR_STR[];
// symbol *quote = symbol_builtin_create(QUOTE_STR, bi_quote, 1);
extern char DEFMACRO_STR[];
extern char MACROEXPAND_STR[];
/* __USER_DATA char QUASIQUOTE_STR[] = "quasiquote"; */
/* __USER_DATA char COMMA_STR[] = "comma"; */
/* __USER_DATA char COMMA_AT_STR[] = "comma-at"; */

extern char BOOL_STR_T[];
extern char BOOL_STR_F[];

extern char NIL_STR[];

extern char REST_ARGUMENTS_STR[];

#endif // CONSTANTS_H_
