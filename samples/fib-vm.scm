(define fib ())

(print
 (eval-vm
  '((lambda ()
      (set! fib
            (lambda (n)
              (if (= 0 n)
                0
                (if (= 1 n)
                  1
                  (+ (fib (- n 1)) (fib (- n 2)))))))
      (fib 30)))))
(newline)

;; first heap based vm version
;;
;; $ time ./lisp < samples/fib-vm.scm
;; 832040
;; ./lisp < samples/fib-vm.scm  6.69s user 1.98s system 116% cpu 7.417 total
;;
;;
;; stack based vm version (4.1)
;;
;; $ time ./lisp < samples/fib-vm.scm
;; 832040
;; ./lisp < samples/fib-vm.scm  4.48s user 1.37s system 120% cpu 4.855 total
;;
;;
;; naive interpreter version
;;
;; $ time ./lisp < samples/fib.scm
;; 832040
;; ./lisp < samples/fib.scm  4.16s user 0.52s system 104% cpu 4.454 total
