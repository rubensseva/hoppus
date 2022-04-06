(defun (add x y)
  (+ x y))

(defun (sub x y)
    (- x y))

(defun (foo x y z)
  (sub (add x (+ y 10)) (+ x y 1)))

(define test-result (progn
  (if (sub 1 1)
      (foo 1 2 3)
      (foo 5 6 7))))

(if (eq test-result 9)
    (print "passed arith test")
    (print "failed arith test..."))

(if (eq "hello" "hello")
    (print "passed string test")
    (print "failed string test..."))

(defmacro (mac-defun sym)
    (list 'defun (list sym 'x 'y) '(+ x y)))

(mac-defun mac-add)
(if (eq (mac-add 3 4) 7)
    (print "passed macro test")
    (print "failed macro test..."))

(defmacro (mac-add-many &rest rst)
  `(+ ,@rst))

(if (eq (mac-add-many 1 2 3 4) 10)
    (print "passed quasi/comma-at test")
    (print "failed quasi/comma-at test..."))
