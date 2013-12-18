; LISP subset compiler
;
; compiles function applications with constant
; or nested function call arguments
;
; see B.A. Pumplin, "Compiling LISP Procedures"
; ACM SIGART Newsletter 99, January, 1987

; primary functions

(set 'compexp
    '(lambda (exp)
	(cond ((isconst exp) (list (mksend 1 exp)))
	    (t (compapply (func exp)
		    (complis (arglist exp))
		    (length (arglist exp)))))))

(set 'complis
    '(lambda (u)
	(cond ((null u) '())
	    ((null (rest u)) (compexp (first u)))
	    (t (append-3 (compexp (first u))
		    (list (mkalloc 1))
		    (complis (rest u)))))))

(set 'compapply
    '(lambda (fn vals n)
	(append-3 vals (mklink n) (list (mkcall fn)))))


; recognizer function

(set 'isconst
    '(lambda (x)
	(or (numberp x) (eq x t) (eq x nil)
	    (and (not (atom x)) (eq (first x) 'quote)))))


; selector functions
(set 'func '(lambda (x) (first x)))
(set 'arglist '(lambda (x) (rest x)))


; constructor functions
; (code generator)
(set 'mksend '(lambda (dest val) (list 'MOVEI dest val)))
(set 'mkalloc '(lambda (dest) (list 'PUSH 'sp dest)))
(set 'mkcall '(lambda (fn) (list 'CALL fn)))
(set 'mklink
    '(lambda (n) 
	(cond ((eqn n 1) '())
	    (t (concat (mkmove n 1) (mklink1 (sub1 n)))))))
(set 'mklink1
    '(lambda (n)
	(cond ((zerop n) '())
	    (t (concat (mkpop n) (mklink1 (sub1 n)))))))
(set 'mkpop '(lambda (n) (list 'POP 'sp n)))
(set 'mkmove '(lambda (dest val) (list 'MOVE dest val)))


; auxiliary functions
(set 'first '(lambda (x) (car x)))
(set 'rest '(lambda (x) (cdr x)))
(set 'concat
    '(lambda (element sequence)
	(cond ((listp sequence) (cons element sequence))
	    (t '()))))
(set 'append-3
    '(lambda (l1 l2 l3)
	(append l1 (append l2 l3))))
(set 'listp
    '(lambda (x)
	(cond ((consp x) t) ((null x) t) (t nil))))

; not built in to walk
(set 'consp '(lambda (e) (not (atom e))))
(set 'eqn '(lambda (x y) (eq x y)))
