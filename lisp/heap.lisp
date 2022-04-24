(defun heap-create ()
    nil)

(defun get-premade-heap ()
    (list 1 2 3 4 5 6 7 8 9))

(defun heap-insert (heap el)
    (concat heap (list el)))

(defun heap-child-left (i)
    (+ (* 2 i) 1))

(defun heap-child-right (i)
    (+ (* 2 i) 1))

(defun heap-parent (i)
    (/ (- i 1) 2))


(defun print-heap (heap)
    (__print-heap heap 0))


(defun __heap-swap-handle-entry (heap i j curr-i)
    (cond
      ((eq curr-i i) (nth j heap))
      ((eq curr-i j) (nth i heap))
      (true (nth curr-i heap))))

(defun __heap_swap (heap i j curr-heap curr-i)
  (if (eq curr-i (length heap))
      curr-heap
      (__heap_swap heap i j
                   (concat curr-heap (list (__heap-swap-handle-entry heap i j curr-i)))
                   (+ curr-i 1))))

(defun heap-swap (heap i j)
    (__heap_swap heap i j nil 0))


(defun __heap-balance-follow (heap curr-i)
  (cond ((or (lt curr-i 0) (eq curr-i 0)) heap)
        ((lt (nth (heap-parent curr-i) heap) (nth curr-i heap))
         (__heap-balance-follow (heap-swap heap (heap-parent curr-i) curr-i) (heap-parent curr-i)))
        (true (__heap-balance-follow heap (heap-parent curr-i)))))

(defun heap-balance-last-node (heap)
  (__heap-balance-follow heap (- (length heap) 1)))


(defun __heap-balance-all (heap curr-i)
    (if (or (lt curr-i 0) (eq curr-i 0))
          heap
          (__heap-balance-all (__heap-balance-follow heap curr-i) (- curr-i 1))))

(defun heap-balance-all (heap)
    (if (not (eq heap (__heap-balance-all heap (- (length heap) 1))))
        (heap-balance-all (__heap-balance-all heap (- (length heap) 1)))
        (__heap-balance-all heap (- (length heap) 1))))

(heap-balance-all (get-premade-heap))
