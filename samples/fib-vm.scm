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
;; ./lisp < samples/fib-vm.scm  6.69s user 1.98s system 116% cpu 7.417 total
;;
;; naive interpreter version
;;
;; $ time ./lisp < samples/fib.scm
;; 832040
;; ./lisp < samples/fib.scm  4.16s user 0.52s system 104% cpu 4.454 total
