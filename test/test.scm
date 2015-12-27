(define p (lambda (x)
            (print x)
            (newline)))

(define assert
  (lambda (expected actual msg)
    (print (if (equal? expected actual) 'success_ 'failed_) msg)
    (if (not (equal? expected actual))
      (print '__ expected '_expected_but_got_ actual))
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

(assert #t (number? 1) 'number?_0)
(assert #f (number? 'a) 'number?_1)
(assert #t (integer? 1) 'integer?_0)
(assert #f (integer? 'a) 'integer?_1)
(assert #t (zero? 0) 'zero?_0)
(assert #f (zero? 1) 'zero?_1)

(assert #f (positive? (- 1)) 'positive?_0)
(assert #f (positive? 0) 'positive?_1)
(assert #t (positive? 1) 'positive?_2)
(assert #t (negative? (- 1)) 'negative?_0)
(assert #f (negative? 0) 'negative?_1)
(assert #f (negative? 1) 'negative?_2)
(assert #t (odd? (- 1)) 'odd?_0)
(assert #f (odd? 0) 'odd?_1)
(assert #t (odd? 1) 'odd?_2)
(assert #f (even? (- 1)) 'even?_0)
(assert #t (even? 0) 'even?_1)
(assert #f (even? 1) 'even?_2)

(assert #t (equal? '(1 2 3) '(1 2 3)) 'equal?_0)
(assert #t (equal? '((1 2) (3 4)) '((1 2) (3 4))) 'equal?_1)
(assert #f (equal? '(1 2 3) 'a) 'equal?_2)
(assert #t (equal? 'a 'a) 'equal?_3)
(assert #t (equal? 1 1) 'equal?_4)

(assert 55 (apply + '(1 2 3 4 5 6 7 8 9 10)) 'apply_0)
(assert '((1 2 3) 4 5 6) (apply list '(1 2 3) '(4 5 6)) 'apply_1)
(assert 6 (apply (lambda (a b c) (+ a b c)) '(1 2 3)) 'apply_2)

(assert #t (and) 'and_0)
(assert 3 (and 1 2 3) 'and_1)
(assert #f (and 1 #f (print 'failed_and_2)) 'and_2)

(assert #f (or) 'or_0)
(assert 1 (or #f #f 1) 'or_1)
(assert 1 (or #f 1 (print 'failed_or_2)) 'or_2)

(assert 0 (let () 0) 'let_0)
(assert 1 (let ((a 1)) a) 'let_1)
(assert 2 (let ((a 1) (b 1)) (+ a b)) 'let_2)

(define x 1)
(set! x 2)
(assert 2 x 'set!_0)

(define y 2)
(define x
  (lambda (y)
    (set! y 1)
    y))
(assert 1 (x 3) 'set!_1)
(assert 2 y 'set!_2)

(assert 2 (/ 12 2 3) 'div_0)

(define p (cons 1 2))
(set-car! p 3)
(assert '(3 . 2) p 'set-car!_0)
(set-cdr! p 4)
(assert '(3 . 4) p 'set-cdr!_0)

(assert #t (vector? (make-vector 5)) 'vector_0)
(define x (make-vector 5))
(vector-set! x 0 1)
(assert 1 (vector-ref x 0) 'vector_1)
