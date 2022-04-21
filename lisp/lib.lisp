(defmacro not (form)
    `(if ,form false true))

(defun when (pred form)
    (if pred form nil))

(defun cadr (cell)
    (car (cdr cell)))

(defun list (&rest rst)
    rst)

(defun __length (list count)
    (if list
        (__length (cdr list) (+ count 1))
        count))

(defun length (list)
    (if list
        (__length list 0)
        0))

(defun concat (seq1 seq2)
    (if seq1
    (cons (car seq1) (concat (cdr seq1) seq2))
    seq2))


(defun nth (i l)
  (if i (nth (- i 1) (cdr l))
        (car l)))

(defun first (l)
  (car l))
(defun second (l)
  (nth 1 l))
(defun third (l)
  (nth 2 l))
(defun fourth (l)
  (nth 3 l))
(defun fifth (l)
  (nth 4 l))
