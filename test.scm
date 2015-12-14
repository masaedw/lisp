(define p (lambda (x)
            (print x)
            (newline)))

(define assert
  (lambda (expected actual msg)
    (print (if (eqv? expected actual) 'success_ 'failed_) msg)
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


(assert #t (list? '(a)) 'list_0)
(assert #t (list? '(a b c)) 'list_1)
(assert #f (list? '()) 'list_2)
(assert #f (list? 'a) 'list_3)
(assert #f (list? '(a . b)) 'list_4)
(assert #f (list? 1) 'list_5)


(define x
  (lambda (a . args)
    (if (list? args)
      (length args)
      (if (null? args)
        'nil
        'error))))

(define y
  (lambda args
    (if (list? args)
      (length args)
      (if (null? args)
        'nil
        'error))))


(assert 2 (x 'a 'b 'c) 'rest_aprams_0)
(assert 'nil (x 'a) 'rest_params_1)

(assert 3 (y 'a 'b 'c) 'rest_aprams_2)
(assert 'nil (y) 'rest_params_3)

(assert 0 (+) 'plus_0)
(assert 1 (+ 1) 'plus_1)
(assert 2 (+ 1 1) 'plus_2)
(assert 3 (+ 1 1 1) 'plus_3)

(assert (- 0 5) (- 5) 'minus_0)
(assert 0 (- 5 5) 'minus_1)
(assert 3 (- 5 1 1) 'minus_2)
