(define map-1
  (lambda (proc seq)
    (if (null? seq)
      '()
      (cons (proc (car seq))
            (map-1 proc (cdr seq))))))

(define map
  (lambda (proc . arglists)
    (if (null? (car arglists))
      '()
      (cons
       (apply proc (map-1 car arglists))
       (apply map
              (cons proc (map-1 cdr arglists)))))))

(print (map list '(1 2 3) '(4 5 6) '(7 8 9)))
(newline)
