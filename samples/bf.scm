(define (abort msg)
  (print msg)
  (sys-exit 1))

(define bfc-inc 62) ; >   ++ptr
(define bfc-dec 60) ; <   --ptr
(define bfc-add 43) ; +   ++(*ptr)
(define bfc-sub 45) ; -   --(+ptr)
(define bfc-puc 46) ; .   putchar(*ptr)
(define bfc-gec 44) ; ,   *ptr = getchar()
(define bfc-beg 91) ; [   while(*ptr) {
(define bfc-end 93) ; ]   }

(define bf-chars
  (list bfc-inc
        bfc-dec
        bfc-add
        bfc-sub
        bfc-puc
        bfc-gec
        bfc-beg
        bfc-end))

(define (bf-insn-char? c)
  (memv c bf-chars))

(define (bf-parse in)
  (let1 c (read-char in)
    (cond
     ((eof-object? c) ())
     ((bf-insn-char? c) (cons c (bf-parse in)))
     (else (bf-parse in)))))

(define i-pinc  1) ; pointer increment
(define i-pdec  2) ; pointer decrement
(define i-vinc  3) ; value increment
(define i-vdec  4) ; value decrement
(define i-ichr  5) ; input char
(define i-ochr  6) ; output char
(define i-beqz  7) ; branch equal zero
(define i-jmpf  8) ; jump forward
(define i-jmpb  9) ; jump backword
(define i-jump 10) ; jump
(define i-zero 11) ; set pointer to zero [-]

(define (reverse xs)
  (define (iter x xs)
    (if (null? xs)
      x
      (iter (cons (car xs) x) (cdr xs))))
  (iter () xs))

(define (bf-rle xs)
  (define (iter a c x xs)
    (cond
     ((null? xs)
      (reverse (acons a c x)))
     ((or (= (car xs) bfc-beg) (= (car xs) bfc-end))
      (iter (car xs) 1 (acons a c x) (cdr xs)))
     ((= (car xs) a)
      (iter a (+ c 1) x (cdr xs)))
     (else
      (iter (car xs) 1 (acons a c x) (cdr xs)))))

  (if (null? xs)
    ()
    (iter (car xs) 1 () (cdr xs))))

(define (bf-compile xs)
  (vector (bf-rle (bf-parse xs))))

(define pc 0)
(define p 0)
(define (inc-pc) (set! pc (+ pc 1)))
(define mem (make-vector 30000 0))

(define (current-insn prog)
  (vector-ref prog pc))
(define (current-memory)
  (vector-ref mem p))
(define (set-current-memory! v)
  (vector-set! mem p v))

(define (jump-forward prog tpc n)
  (if (= n 0)
    (set! pc tpc)
    (let1 i (car (vector-ref prog (+ tpc 1)))
      (cond
       ((= i bfc-beg)
        (jump-forward prog (+ tpc 1) (+ n 1)))
       ((= i bfc-end)
        (jump-forward prog (+ tpc 1) (- n 1)))
       (else
        (jump-forward prog (+ tpc 1) n))))))

(define (jump-backward prog tpc n)
  (if (= n 0)
    (set! pc (- tpc 1))
    (let1 i (car (vector-ref prog (- tpc 1)))
      (cond
       ((= i bfc-end)
        (jump-backward prog (- tpc 1) (+ n 1)))
       ((= i bfc-beg)
        (jump-backward prog (- tpc 1) (- n 1)))
       (else
        (jump-backward prog (- tpc 1) n))))))


(define (iterate n x)
  (if (<= n 0)
    ()
    (begin
      (x)
      (iterate (- n 1) x))))

(define-macro repeat
  (lambda (n . body)
    (list 'iterate n (cons 'lambda (cons () body)))))

(define (bf-vm in out prog)
  (define (iter)
    (if (> (vector-length prog) pc)
      (let ((i (car (current-insn prog)))
            (c (cdr (current-insn prog))))
        (cond
         ((= i bfc-inc)
          (set! p (+ p c)))
         ((= i bfc-dec)
          (set! p (- p c)))
         ((= i bfc-add)
          (set-current-memory! (+ (current-memory) c)))
         ((= i bfc-sub)
          (set-current-memory! (- (current-memory) c)))
         ((= i bfc-puc)
          (repeat c (write-u8 (current-memory) out)))
         ((= i bfc-gec)
          (repeat c (set-current-memory! (let1 c (read-u8 in)
                                           (if (eof-object? c) 0 c)))))
         ((= i bfc-beg)
          (if (= (current-memory) 0)
            (jump-forward prog pc 1)))
         ((= i bfc-end)
          (jump-backward prog pc 1)))
        (inc-pc)
        (iter))))
  (iter))

(define (main args)
  (let1 file (open-input-file (car (cdr args)))
    (bf-vm (current-input-port) (current-output-port) (bf-compile file))))

(main (command-line))
