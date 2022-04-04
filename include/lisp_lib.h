#ifndef LISP_LIB_H_
#define LISP_LIB_H_

#define LISP_LIB_WHEN          \
    "(defun (when pred form) " \
    "(if pred form nil))"

#define LISP_LIB_CADR          \
    "(defun (cadr cell) "      \
    "(car (cdr cell)))"

#define LISP_LIB_LIST          \
    "(defun (list &rest rst) " \
    "rst)"

#define LISP_LIB__LENGTH                   \
    "(defun (__length list count) "        \
    "(if list "                            \
    "(__length (cdr list) (+ count 1)) "   \
    "count))"

#define LISP_LIB_LENGTH           \
    "(defun (length list) "       \
    "(if list "                   \
    "(+ 1 (__length list 0)) "    \
        "0)) "

#define LISP_LIB_CONCAT                             \
    "(defun (concat seq1 seq2) "                    \
    "(if seq1 "                                     \
    "(cons (car seq1) (concat (cdr seq1) seq2)) "   \
    "seq2))"

const char* lib_strs[] = {
    LISP_LIB_WHEN,
    LISP_LIB_CADR,
    LISP_LIB_LIST,
    LISP_LIB__LENGTH,
    LISP_LIB_LENGTH,
    LISP_LIB_CONCAT,
};


#endif // LISP_LIB_H_
