#ifndef LISP_LIB_H_
#define LISP_LIB_H_

#include <link.h>

__USER_DATA char LISP_LIB_IF[] = \
    "(defmacro if (pred form other-form) " \
    "`(cond (,pred ,form) (true ,other-form))) ";

__USER_DATA char LISP_LIB_NOT[] =           \
    "(defmacro not (form) "    \
    "`(if ,form false true)) ";

__USER_DATA char LISP_LIB_WHEN[] =                       \
    "(defmacro when (pred form) "   \
    "`(if ,pred ,form nil)) ";

__USER_DATA char LISP_LIB_CADR[] =          \
    "(defun cadr (cell) "      \
    "(car (cdr cell)))";

__USER_DATA char LISP_LIB_LIST[] =          \
    "(defun list (&rest rst) " \
    "rst)";

__USER_DATA char LISP_LIB__LENGTH[] =                   \
    "(defun __length (list count) "        \
    "(if list "                            \
    "(__length (cdr list) (+ count 1)) "   \
    "count))";

__USER_DATA char LISP_LIB_LENGTH[] =           \
    "(defun length (list) "       \
    "(if list "                   \
    "(__length list 0) "    \
    "0)) ";

__USER_DATA char LISP_LIB_CONCAT[] =                             \
    "(defun concat (seq1 seq2) "                    \
    "(if seq1 "                                     \
    "(cons (car seq1) (concat (cdr seq1) seq2)) "   \
    "seq2))";

__USER_DATA char LISP_LIB_NTH[] =                \
    "(defun nth (i l) "             \
     "(if i (nth (- i 1) (cdr l)) " \
    "(car l))) ";

__USER_DATA char LISP_LIB_FIRST[] =      \
    "(defun first (l) "     \
    "(car l)) ";
__USER_DATA char LISP_LIB_SECOND[] =     \
    "(defun second (l) "    \
    "(nth 1 l)) ";
__USER_DATA char LISP_LIB_THIRD[] =      \
    "(defun third (l) "     \
    "(nth 2 l)) ";
__USER_DATA char LISP_LIB_FOURTH[] =     \
    "(defun fourth (l) "    \
    "(nth 3 l)) ";
__USER_DATA char LISP_LIB_FIFTH[] =      \
    "(defun fifth (l) "     \
    "(nth 4 l)) ";

__USER_DATA char LISP_LIB_FIB[] =  \
"(defun fib (n)" \
    "(if (lt n 2) n" \
    "(+ (fib (- n 1)) (fib (- n 2)))))";

__USER_DATA const char* lib_strs[] = {
    LISP_LIB_IF,
    LISP_LIB_NOT,
    LISP_LIB_WHEN,
    LISP_LIB_CADR,
    LISP_LIB_LIST,
    LISP_LIB__LENGTH,
    LISP_LIB_LENGTH,
    LISP_LIB_CONCAT,
    LISP_LIB_NTH,
    LISP_LIB_FIRST,
    LISP_LIB_SECOND,
    LISP_LIB_THIRD,
    LISP_LIB_FOURTH,
    LISP_LIB_FIFTH,
    LISP_LIB_FIB,
};


#endif // LISP_LIB_H_
