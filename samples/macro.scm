(define-macro m
  (lambda ()
    ''macro))

(define-macro my-if
  (lambda (condc thenc elsec)
    (list 'if
          condc
          thenc
          elsec)))

(define x
  (macroexpand '(my-if #t (my-if #f a b) (m))))

(print x)
