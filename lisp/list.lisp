(defun (__length list count)
  (if list
      (__length (cdr list) (+ count 1))
      count))

(defun (length list)
    (if list
        (+ 1 (__length list 0))
        0))
