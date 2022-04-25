#ifndef LISP_LIB_H_
#define LISP_LIB_H_

#define LISP_LIB_IF                          \
    "(defmacro if (pred form other-form) "   \
    "`(cond (,pred ,form) (true ,other-form))) "

#define LISP_LIB_NOT           \
    "(defmacro not (form) "    \
    "`(if ,form false true)) "

#define LISP_LIB_WHEN               \
    "(defmacro when (pred form) "   \
    "`(if ,pred ,form nil)) "

#define LISP_LIB_CADR          \
    "(defun cadr (cell) "      \
    "(car (cdr cell)))"

#define LISP_LIB_LIST          \
    "(defun list (&rest rst) " \
    "rst)"

#define LISP_LIB__LENGTH                   \
    "(defun __length (list count) "        \
    "(if list "                            \
    "(__length (cdr list) (+ count 1)) "   \
    "count))"

#define LISP_LIB_LENGTH           \
    "(defun length (list) "       \
    "(if list "                   \
    "(__length list 0) "    \
        "0)) "

#define LISP_LIB_CONCAT                             \
    "(defun concat (seq1 seq2) "                    \
    "(if seq1 "                                     \
    "(cons (car seq1) (concat (cdr seq1) seq2)) "   \
    "seq2))"

#define LISP_LIB_NTH                \
    "(defun nth (i l) "             \
     "(if i (nth (- i 1) (cdr l)) " \
        "(car l))) "

#define LISP_LIB_FIRST      \
    "(defun first (l) "     \
    "(car l)) "
#define LISP_LIB_SECOND     \
    "(defun second (l) "    \
    "(nth 1 l)) "
#define LISP_LIB_THIRD      \
    "(defun third (l) "     \
    "(nth 2 l)) "
#define LISP_LIB_FOURTH     \
    "(defun fourth (l) "    \
    "(nth 3 l)) "
#define LISP_LIB_FIFTH      \
    "(defun fifth (l) "     \
    "(nth 4 l)) "


const char* lib_strs[] = {
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
};


#endif // LISP_LIB_H_
