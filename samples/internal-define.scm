(define var 1)


(define i
  '(define x
     (lambda ()
       (define var 2)
       (define v2 3)
       var)))

(print (compile i 'next))
(eval-vm i)

(print (x))
(print var)

(define y
  (lambda (a b)
    (define c 3)
    (define d 4)
    (list a b c d)))

(print (y 1 2))

(define z
  (lambda (a b)
    (define c 3)
    ;(define c 4) ;; error
    (list a b c)))
