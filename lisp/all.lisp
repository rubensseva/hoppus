(defun (add x y)
  (+ x y))

(defun (sub x y)
    (- x y))

(defun (foo x y z)
  (sub (add x (+ y 10)) (+ x y 1)))

(progn
  (if (sub 1 1)
      (foo 1 2 3)
      (foo 5 6 7)))
