(define and
  (lambda (a b)
    (if a
      (if b
        #t
        #f)
      #f)))

(define pn print)

(define fizzbuzz-sub
  (lambda (s max f b)
    (if (eqv? s max)
      ()
      (if (and (eqv? f 3) (eqv? b 5))
        (begin
          (pn 'FizzBuzz)
          (fizzbuzz-sub (+ s 1) max 1 1))
        (if (eqv? f 3)
          (begin
            (pn 'Fizz)
            (fizzbuzz-sub (+ s 1) max 1 (+ b 1)))
          (if (eqv? b 5)
            (begin
              (pn 'Buzz)
              (fizzbuzz-sub (+ s 1) max (+ f 1) 1))
            (begin
              (pn s)
              (fizzbuzz-sub (+ s 1) max (+ f 1) (+ b 1)))))))))

(define fizzbuzz
  (lambda (max)
    (fizzbuzz-sub 1 (+ 1 max) 1 1)))

(fizzbuzz 15)
