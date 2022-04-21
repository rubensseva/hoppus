(defun fib (n)
    (if (lt n 2) n
        (+ (fib (- n 1)) (fib (- n 2)))))


(if (eq (fib 1) 1) (print "1 passed") (print "1 fail!"))
(if (eq (fib 12) 144) (print "144 passed") (print "144 fail!"))
