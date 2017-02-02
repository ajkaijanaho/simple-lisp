(define
  (fact (lambda (x)
          (cond
           ( (eq x 0) 1)
           ( 't       (mul x (fact (sub x 1))))))))
(print (fact 5))
