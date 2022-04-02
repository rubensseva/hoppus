# LISP
This LISP is made as part of a master thesis on microkernels. The plan
is to run this interpreter on the microkernel I am working on.

# DOC (in progress)
## BUILTINS
### + add
Adds all arguments
```
(+ 1 2 3 4)  =>   10
```

### - sub
Subtract arguments from the first argument
```
(+ 10 4 3)  =>   3
```

### cons
Create a cons cell
```
(cons 1 2)    =>   (1 2)
```

### car
Get the first element of cons cell

```
(car (cons 1 2))    =>    1
```

### cdr
Get the second element of cons cell

```
(cdr (cons 1 2))    =>    2
```

### progn
Evaluate the arguments one at a time, return the result of the evaluation
of the last argument

```
(progn (+ 1 2) (+ 3 4))      =>     7
(progn (print 3) (+ 1 3))    =>     3 is printed, 4 is returned
```

### if
Accepts three arguments.
If the first argument is true, evaluate the second argument and return result of evaluation.
If the first argument is false, evaluate the third argument and return result of evaluation.

```
(if (- 1 1) (+ 1 2) (+ 3 4))   =>    7
```

### print
Print an expression

```
(print 4)                    =>     4 is printed
(progn (print 3) (+ 1 3))    =>     3 is printed, 4 is returned
```


## REPL
If you input some code with unmatched opening parentheses, the REPL manages to
wait until you complete the expression, but it can't handle a newline in the
middle of a symbol.

## LISTS
Lists are made up of cons cells, where the `cdr` field points to the next cons cell.
The end of a list is signified by a cons cell with `car` and `cdr` fields set to `nil`.
An empty list is a cons cell with `car` and `cdr` fields set to `nil`.

## NULL/nil
`nil` is represented by using C's `NULL` construct. Internally, an expression (`expr *`)
is `nil` if it points to `NULL`.

## BOOLEANS
Boolean values are represented with the BOOLEAN type, where the `.data` field of the
`expr` is set to 1 for true, and 0 for false.

### Boolean values for other types
- NUMBERS: 1 is true, 0 is false
- CHAR: Same as numbers
- BOOLEANS: `true` is true, `false` is false. Internally, true and false are differentiated with
  the `data` field in `expr`, 1 in the data field means true, 0 means false.
- CONS CELLS: If a cons cell has both the `car` and the `cdr` field set to `nil`, it will
  be regarded as false. This implies that an empty list (also a cons cell with NULL `car`
  and `cdr` fields) is also false.

## STRINGS
Strings are represented with lists, there is no special "string" type. If you input
a value such as `"hello world"`, it will  turn it into a list using cons cells, where
each element is a char. This way you can use the familiar list functions to operate
on lists, such as `car` and `cdr`.

## CHARS
Currently, char literals are not implemented. You can get a char by running the `car`
function on a string, for example: `(car "h")`
