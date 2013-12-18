; numeric functions
; 
; This is all symbol manipulation- walk has no built-in math!
; It's too slow to be of much use.

; error conditions
(set 'NOADD '"addition undefined for given arguments")
(set 'NOSUB '"subtraction undefined for given arguments")

(set '_integers_ '(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16))

; increment a number by 1
(set 'add1 '(lambda (n) (or (succ n _integers_) (error NOADD))))

; decrement by 1
(set 'sub1 '(lambda (n) (or (pred n _integers_) (error NOSUB))))

; t if x is 0
(set 'zerop '(lambda (x) (eq x 0)))

; t if x is a number
(set 'numberp '(lambda (x) (member x _integers_)))

; t if x > y
(set 'greaterp '(lambda (x y) (member x (after _integers_ y))))

; t if x < y
(set 'lessp '(lambda (x y) (member y (after _integers_ x))))

; subtraction
(set 'difference
    '(lambda (x y)
	(cond ((zerop y) x)
	    (t (difference (sub1 x) (sub1 y))))))

; take the length of a list
(set 'length
    '(lambda (s)
	(cond ((atom s) 0)  ; or (null s)
	    ((null (cdr s)) 1)
	    (t (add1 (length (cdr s)))))))

; addition (forget about actually using this)
(set 'plus
    '(lambda (x y)
	(length (append (before _integers_ x) (before _integers_ y)))))
