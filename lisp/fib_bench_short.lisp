(defun fib (n)
    (if (lt n 2) n
        (+ (fib (- n 1)) (fib (- n 2)))))

(fib 30)
