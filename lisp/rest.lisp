(defun (add &rest rst)
    (+ (car rst) (car (cdr rst))))

(add 1 2)
