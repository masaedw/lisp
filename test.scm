(define p (lambda (x)
            (print x)
            (newline)))

(p (if (eqv? 1 1)
     'success_eqv_1_1
     'failed_eqv_1_1))

(define-macro m
  (lambda (f)
    ''macro))

(p (if (eq? (m) 'macro)
     'success_define-macro
     'failed_define-macro))
