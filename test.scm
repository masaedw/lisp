(define p (lambda (x)
            (print x)
            (newline)))

(p (if (eq? 1 1)
     'success_eq_1_1
     'failed_eq_1_1))

