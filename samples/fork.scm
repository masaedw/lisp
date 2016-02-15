;https://github.com/shirok/Gauche/blob/master/test/system.scm#L586

;; (test* "fork & pipe" 70000
;;        (receive (in out) (sys-pipe)
;;          (let1 pid (sys-fork)
;;            (if (= pid 0)
;;              (begin (close-input-port in)
;;                     (display (make-string 69999) out)
;;                     (with-error-handler
;;                         (^e (sys-exit 0))
;;                       (lambda ()
;;                         (newline out)
;;                         (close-output-port out)
;;                         (sys-pause))))
;;              (let loop ((toread 70000)
;;                         (nread  0))
;;                (let1 r (string-size (read-block toread in))
;;                  (if (>= (+ nread r) 70000)
;;                    (begin (sys-kill pid SIGTERM)
;;                           (sys-waitpid pid)
;;                           (+ nread r))
;;                    (loop (- toread r) (+ nread r)))))
;;              ))))

(let ((r (sys-pipe))
      (test "hogehoge"))
  (let ((in (car r))
        (out (cdr r)))
    (let ((pid (sys-fork)))
      (if (= pid 0)
        (let ()
          (close-port in)
          (display test out)
          (newline out)
          (close-port out)
          (sys-pause))
        (let ((r (read-line in)))
          (print (string=? r test))
          (sys-kill pid SIGTERM)
          (sys-waitpid pid))))))
          







