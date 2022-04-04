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

(if (equal test-result 7)
    (print "passed arith test!")
    (print "failed arith test..."))

(if (equal "hello" "hello")
    (print "passed string test!")
    (print "failed string test..."

(defmacro (mac-defun sym)
    (list 'defun (list sym 'x 'y) '(+ x y)))

(macroexpand (mac-defun test2))
(mac-defun mac-add)
(if (equal (mac-add 3 4) 7)
    (print "passed macro test!")
    (print "failed macro test..."))
