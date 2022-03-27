##
# Project Title
#
# @file
# @version 0.1

build/ukernel_lisp.out: main.c src/tokenize.c src/eval.c src/ir.c src/utility.c include/tokenize.h include/eval.h include/ir.h include/utility.h
	gcc -g main.c src/tokenize.c src/eval.c src/ir.c src/utility.c -Iinclude -o build/ukernel_lisp.out

# end
