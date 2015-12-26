(eval-vm
 '(define fib
    (lambda (n)
      (if (= 0 n)
        0
        (if (= 1 n)
          1
          (+ (fib (- n 1)) (fib (- n 2))))))))

(print (eval-vm '(fib 30)))
(newline)

;; first heap based vm version
;;
;; $ time ./lisp < samples/fib-vm.scm
;; 832040
;; ./lisp < samples/fib-vm.scm  6.37s user 1.59s system 111% cpu 7.102 total
;;
;; naive interpreter version
;;
;; $ time ./lisp < samples/fib.scm
;; 832040
;; ./lisp < samples/fib.scm  5.03s user 1.22s system 115% cpu 5.439 total
