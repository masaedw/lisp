(define assert
  (lambda (expected actual msg)
    (display (if (equal? expected actual) 'success_ 'failed_))
    (display msg)
    (if (not (equal? expected actual))
      (print '__ expected '_expected_but_got_ actual)
      (newline))))

(print (if (eqv? 1 1)
         'success_eqv_1_1
         'failed_eqv_1_1))

(define-macro m
  (lambda ()
    ''macro))

(define-macro my-if
  (lambda (condc thenc elsec)
    (list 'if
          condc
          thenc
          elsec)))

(print (if (eq? (m) 'macro)
         'success_define-macro
         'failed_define-macro))

(my-if #t
       (print 'success_my-if)
       (print 'failed_my-if))

(assert ''macro (macroexpand '(m)) 'macroexpand_0)
(assert '(if #t (if #f a b) 'macro) (macroexpand '(my-if #t (my-if #f a b) (m))) 'macroexpand_1)

(assert 'a (append 'a) 'append_0)
(assert '(a . b) (append '(a) 'b) 'append_1)
(assert '(a b c d) (append '(a b) '(c d)) 'append_2)

(assert #t (symbol? 'a) 'symbol?_0)
(assert #f (symbol? 1) 'symbol?_1)
(assert #t (symbol=? 'a 'a 'a) 'symbol=?_0)
(assert #f (symbol=? 'a 'a 'b) 'symbol=?_1)

(assert "a" (symbol->string 'a) 'symbol->string_0)
(assert 'a (string->symbol "a") 'string->symbol_0)

(assert #t (list? '(a)) 'list_0)
(assert #t (list? '(a b c)) 'list_1)
(assert #t (list? '()) 'list_2)
(assert #f (list? 'a) 'list_3)
(assert #f (list? '(a . b)) 'list_4)
(assert #f (list? 1) 'list_5)

(assert #t (dotted-list? 'a) 'dotted-list?_0)
(assert #t (dotted-list? '(a . a)) 'dotted-list?_1)
(assert #t (dotted-list? '(a a . a)) 'dotted-list?_2)
(assert #f (dotted-list? '()) 'dotted-list?_3)
(assert #f (dotted-list? '(a)) 'dotted-list?_4)
(assert #f (dotted-list? '(a a)) 'dotted-list?_5)

(define x
  (lambda (a . args)
    (if (null? args)
      'nil
      (if (list? args)
        (length args)
        'error))))

(define y
  (lambda args
    (if (null? args)
      'nil
      (if (list? args)
        (length args)
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
(assert #t (equal? "hoge" "hoge") 'equal?_5)

(assert 55 (apply + '(1 2 3 4 5 6 7 8 9 10)) 'apply_0)
(assert '((1 2 3) 4 5 6) (apply list '(1 2 3) '(4 5 6)) 'apply_1)
(assert 6 (apply (lambda (a b c) (+ a b c)) '(1 2 3)) 'apply_2)

(assert #t (and) 'and_0)
(assert 1 (and 1) 'and_1)
(assert 3 (and 1 2 3) 'and_2)
(assert #f (and 1 #f (print 'failed_and_2)) 'and_3)

(assert #f (or) 'or_0)
(assert #f (or #f) 'or_1)
(assert 1 (or 1) 'or_2)
(assert 1 (or #f #f 1) 'or_3)
(assert 1 (or #f 1 (print 'failed_or_4)) 'or_4)

(assert 0 (let () 0) 'let_0)
(assert 1 (let ((a 1)) a) 'let_1)
(assert 2 (let ((a 1) (b 1)) (+ a b)) 'let_2)

(define x ())
(let* ((a (begin (set! x (cons 1 x)) x))
       (b (begin (set! x (cons 2 x)) x)))
  #t)
(assert '(2 1) x 'let*_0)

(assert 1 (let1 x 1 x) 'let1_0)

(assert #t (letrec ((odd? (lambda (n) (even? (- n 1))))
                    (even? (lambda (n) (if (= n 0) #t (odd? (- n 1))))))
             (odd? 5))
        'letrec_0)

(define x ())
(letrec* ((a (begin (set! x (cons 1 x)) x))
          (c (begin (set! x (cons 2 x)) x)))
  #t)
(assert '(2 1) x 'letrec*_0)
  

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

(assert '((a . b) 1 2) (acons 'a 'b '(1 2)) 'acons_0)

(define p (cons 1 2))
(set-car! p 3)
(assert '(3 . 2) p 'set-car!_0)
(set-cdr! p 4)
(assert '(3 . 4) p 'set-cdr!_0)

(assert #t (vector? (make-vector 5)) 'vector_0)
(define x (make-vector 5))
(vector-set! x 0 1)
(assert 1 (vector-ref x 0) 'vector_1)
(assert #() (vector ()) 'vector_2)
(assert #(1 2 3) (vector (list 1 (+ 1 1) 3)) 'vector_3)
(assert #(a a a) (make-vector 3 'a) 'vector_4)

(assert #t (set-member? 'a '(a b c)) 'set-member?_0)
(assert #f (set-member? 'd '(a b c)) 'set-member?_1)
(assert '(a b c) (set-cons 'a '(a b c)) 'set-cons_0)
(assert '(d a b c) (set-cons 'd '(a b c)) 'set-cons_1)
(assert '(c b a d e) (set-union '(a b c) '(a d e)) 'set-union_0)
(assert '(b c) (set-minus '(a b c) '(a d e)) 'set-minus_0)
(assert '(a) (set-intersect '(a b c) '(a d e)) 'set-intersect_0)

(assert "" (make-string 0) 'make-string_0)
(assert 9 (string-length (make-string 9)) 'make-string_1)
(assert "abcdefg" (string-append "ab" "cd" "efg") 'string-append_0)
(assert #t (string=? "abc" "abc" "abc") 'string=?_0)
(assert #f (string=? "abc" "abcdefg" "x") 'string=?_1)

(assert 105 (+ 5 (call/cc (lambda (cont) (* 10 10)))) 'call/cc_0)
(assert 25 (+ 5 (call/cc (lambda (cont) (* 10 10 (cont 20))))) 'call/cc_1)

(load "./test/test_sub.scm")
(assert 12345 test-sub 'load_0)

(assert #f (eof-object? #t) 'eof_0)
(assert #t (eof-object? (eof-object)) 'eof_1)

(assert #f (assq 'd '((a . a) (b . b) (c . c))) 'assq_0)
(assert '(a . a) (assq 'a '((a . a) (b . b) (c . c))) 'assq_1)
(assert #f (assv 'd '((a . a) (b . b) (c . c))) 'assv_0)
(assert '(a . a) (assv 'a '((a . a) (b . b) (c . c))) 'assv_1)

(assert #f (memq 'a '()) 'memq_0)
(assert #f (memq 'a '(b c d)) 'memq_1)
(assert '(a d) (memq 'a '(b c a d)) 'memq_2)
(assert #f (memv '1 '()) 'memv_0)
(assert #f (memv '1 '(2 3)) 'memv_1)
(assert '(2 3) (memv '2 '(1 2 3)) 'memv_2)

(assert #f (bytevector? #t) 'bytevector?_0)
(assert #t (bytevector? #u8()) 'bytevector?_1)
(assert #t (bytevector? (make-bytevector 3)) 'make-bytevector_0)
(assert #u8(5 5 5) (make-bytevector 3 5) 'make-bytevector_1)
(assert #u8() (bytevector) 'bytevector_0)
(assert #u8(1 2 3) (bytevector 1 2 3) 'bytevector_1)
(assert 0 (bytevector-length #u8()) 'bytevector-length_0)
(assert 3 (bytevector-length #u8(1 2 3)) 'bytevector-length_1)
(assert 1 (bytevector-u8-ref #u8(1) 0) 'bytevector-u8-ref_0)
(let ((x #u8(1 2 3)))
  (bytevector-u8-set! x 1 5)
  (assert #u8(1 5 3) x 'bytevector-u8-set!_0))
(assert #u8(1 2 3) (bytevector-copy #u8(1 2 3)) 'bytevector-copy_0)
(assert #u8(2 3) (bytevector-copy #u8(1 2 3) 1) 'bytevector-copy_1)
(assert #u8(2) (bytevector-copy #u8(1 2 3) 1 2) 'bytevector-copy_2)
(let ((x #u8(1 2 3 4 5)))
  (bytevector-copy! x 0 #u8(3 2 1))
  (assert #u8(3 2 1 4 5) x 'bytevector-copy!_0)
  (bytevector-copy! x 4 #u8(3 2 1) 2)
  (assert #u8(3 2 1 4 1) x 'bytevector-copy!_1)
  (bytevector-copy! x 1 #u8(1 2 3) 2 3)
  (assert #u8(3 3 1 4 1) x 'bytevector-copy!_3)

  (set! x #u8(1 2 3 4 5 6))
  (bytevector-copy! x 0 x 2 6)
  (assert #u8(3 4 5 6 5 6) x 'bytevector-copy!_4)
  (set! x #u8(1 2 3 4 5 6))
  (bytevector-copy! x 2 x 0 4)
  (assert #u8(1 2 1 2 3 4) x 'bytevector-copy!_5)
  )
(assert #u8() (bytevector-append) 'bytevector-append_0)
(assert #u8(1 2 3) (bytevector-append #u8(1 2 3)) 'bytevector-append_1)
(assert #u8(1 2 3 4 5 6) (bytevector-append #u8(1 2 3) #u8(4 5 6)) 'bytevector-append_2)

(define begin_value 1)

(assert 2 (if #t
            (begin
              (set! begin_value 2)
              2)
            3)
        'begin_0)
(assert 2 begin_value 'begin_1)

(define begin_lambda
  (lambda (a)
    (define a1 1)
    (begin
      (define a2 2)
      (define a3 3))
    (+ a a1 a2 a3)))

(assert 7 (begin_lambda 1) 'begin_2)

(define (add1 a)
  (let1 x 1
    (+ a x)))

(define (add2 a)
  (+ a 2))

(define (add3 a)
  (if #f
    a
    (+ a 3)))

(assert 2 (add1 1) 'define_lambda_0)
(assert 4 (add2 2) 'define_lambda_1)
(assert 6 (add3 3) 'define_lambda_2)

(define (mylist . a) a)

(assert '(a b c) (mylist 'a 'b 'c) 'define_lambda_3)


(define (int-define a)
  (define (x v1 v2)
    (+ v1 v2 5))
  (x 2 (+ a 3)))

(define (int-define2 a)
  (define (y v3 . v4)
    (list v3 v4))
  (y 1 2 3 a))


(assert 11 (int-define 1) 'internal_define_0)
(assert '(1 (2 3 4)) (int-define2 4) 'internal_define_1)

(define (cond-test x)
  (cond
   ((eqv? x 1) 1)
   ((eqv? x 'a) 'a)
   (else ())))

(assert 1 (cond-test 1) 'cond_0)
(assert () (cond-test #f) 'cond_1)
(assert #t (cond ((= 1 1) #t)) 'cond_2)

(define (case-test x)
  (case x
    ((1 2 3) 1)
    ((a b c) 'a)
    (else #f)))

(assert 1 (case-test 1) 'case_0)
(assert #f (case-test 'x) 'case_1)
(assert 1 (case 2 ((1 2 3) 1)) 'case_2)

(assert "test/test.scm" (car (command-line)) 'command-line_0)

(assert #f (get-environment-variable "DID_NOT_DEFINED_ENV") 'get-environment-variable_0)
(define path (get-environment-variable "PATH"))
(assert #t (string? path) 'get-environment-variable_1)
(assert #t (list? (get-environment-variables)) 'get-environment-variables_0)
