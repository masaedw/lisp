(define p (lambda (x)
            (print x)
            (newline)))

(define assert
  (lambda (expected actual msg)
    (print (if (eqv? expected actual) 'success_ 'failed_))
    (p msg)))

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


(assert #t (list? '(a)) 'list_0)
(assert #t (list? '(a b c)) 'list_1)
(assert #f (list? '()) 'list_2)
(assert #f (list? 'a) 'list_3)
(assert #f (list? '(a . b)) 'list_4)
(assert #f (list? 1) 'list_5)
