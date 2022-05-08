#ifndef CONFIG_H_
#define CONFIG_H_

/*
  Configuration variables/constants go here
*/

#define EOF_CODE 11
#define EOF_WHILE_READING_EXPR_ERROR_CODE -11
#define UNBOUND_SYMBOL_NAME_ERROR -2
#define NUMBER_OF_ARGUMENTS_ERROR -21
#define TYPE_ERROR -22
#define INVALID_FORM_ERROR -23

#define GENERIC_ERROR -1

#define TOKEN_STR_MAX_LEN 128
#define TOKENS_MAX_NUM 128
#define SYMBOLS_MAX_NUM 256
#define EXPR_STR_MAX_LEN 128
#define LISP_STR_MAX_LEN 128


#endif // CONFIG_H_
