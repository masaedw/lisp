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
