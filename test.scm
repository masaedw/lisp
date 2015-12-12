(define p (lambda (x)
            (print x)
            (newline)))

(p (if (eq? 1 1)
     'success_eq_1_1
     'failed_eq_1_1))

(define-macro m
  (lambda (f)
    ''macro))

(p (if (eq? (m) 'macro)
     'success_define-macro
     'failed_define-macro))
