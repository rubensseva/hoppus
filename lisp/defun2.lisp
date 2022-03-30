(defun (add x y)
    (+ x y))

(defun (test x y)
  (add (add x x) y))

(test 1 3)
