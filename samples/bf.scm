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

(define (compile xs)
  (apply bytevector xs))

(define pc 0)
(define p 0)
(define (inc-pc) (set! pc (+ pc 1)))
(define mem (make-vector 30000 0))

(define (current-insn prog)
  (bytevector-u8-ref prog pc))
(define (current-memory)
  (vector-ref mem p))
(define (set-current-memory! v)
  (vector-set! mem p v))

(define (jump-forward prog tpc n)
  (if (= n 0)
    (set! pc tpc)
    (let1 i (bytevector-u8-ref prog (+ tpc 1))
      (cond
       ((= i bfc-beg)
        (jump-forward prog (+ tpc 1) (+ n 1)))
       ((= i bfc-end)
        (jump-forward prog (+ tpc 1) (- n 1)))
       (else
        (jump-forward prog (+ tpc 1) n))))))

(define (jump-backward prog tpc n)
  (if (= n 0)
    (set! pc tpc)
    (let1 i (bytevector-u8-ref prog (- tpc 1))
      (cond
       ((= i bfc-end)
        (jump-backward prog (- tpc 1) (+ n 1)))
       ((= i bfc-beg)
        (jump-backward prog (- tpc 1) (- n 1)))
       (else
        (jump-backward prog (- tpc 1) n))))))

(define (bf-vm in out prog)
  (define (iter)
    (if (>= (bytevector-length prog) pc)
      (let1 i (current-insn prog)
        (cond
         ((= i bfc-inc)
          (set! p (+ p 1)))
         ((= i bfc-dec)
          (set! p (- p 1)))
         ((= i bfc-add)
          (set-current-memory! (+ (current-memory) 1)))
         ((= i bfc-sub)
          (set-current-memory! (- (current-memory) 1)))
         ((= i bfc-puc)
          (write-u8 (current-memory) out))
         ((= i bfc-gec)
          (set-current-memory! (let1 c (read-char in)
                                 (if (eof-object? c) 0 c))))
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
    (bf-vm (current-input-port) (current-output-port) (apply bytevector (bf-parse file)))))

(main (command-line))
