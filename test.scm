(define p (lambda (x)
            (print x)
            (newline)))

(p (if (eqv? 1 1)
     'success_eqv_1_1
     'failed_eqv_1_1))

(define-macro m
  (lambda (f)
    ''macro))

(define-macro my-if
  (lambda (condc thenc elsec)
    (list 'if
          condc
          thenc
          elsec)))

(p (if (eq? (m) 'macro)
     'success_define-macro
     'failed_define-macro))

(my-if #t
       (print 'success_my-if)
       (print 'failed_my-if))
(newline)
