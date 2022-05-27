# Hoppus
Hoppus is an experimental LISP interpreter made as part of a master thesis on microkernels. The plan
is to run this interpreter on the microkernel I am working on. Hoppus is a tree-walking interpreter featuring
a garbage collector. 

# DOC (In progress)

## REPL
If you input some code with unmatched opening parentheses, the REPL manages to
wait until you complete the expression, but it can't handle a newline in the
middle of a symbol.

## STRINGS
Strings are represented with lists, there is no special "string" type. If you input
a value such as `"hello world"`, it will  turn it into a list using cons cells, where
each element is a char. This way you can use the familiar list functions to operate
on lists, such as `car` and `cdr`.

## CHARS
Currently, char literals are not implemented. You can get a char by running the `car`
function on a string, for example: `(car "h")`
