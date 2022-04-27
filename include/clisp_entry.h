#ifndef CLISP_ENTRY_H_
#define CLISP_ENTRY_H_

void create_builtins();
void parser_error(int error);
void eval_error(int error);
int load_standard_library();
int REPL_loop(int fd);
int clisp_main();

#endif // CLISP_ENTRY_H_
