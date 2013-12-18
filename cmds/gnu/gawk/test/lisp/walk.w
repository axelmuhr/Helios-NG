; walk.w - extensions to simple interpreter in LISP
;
; Portions copyright (c) 1988, 1990 Roger Rohrbach

; compositions of car, cdr

(set 'cadr '(lambda (e) (car (cdr e))))
(set 'cddr '(lambda (e) (cdr (cdr e))))
(set 'caar '(lambda (e) (car (car e))))
(set 'cdar '(lambda (e) (cdr (car e))))
(set 'cadar '(lambda (e) (car (cdr (car e)))))
(set 'caddr '(lambda (e) (car (cdr (cdr e)))))
(set 'cddar '(lambda (e) (cdr (cdr (car e)))))
(set 'cdadr '(lambda (e) (cdr (car (cdr e)))))

; basic predicates

(set 'null '(lambda (e) (eq e nil)))
(set 'not '(lambda (e) (eq e nil)))

; recursively defined functions

; return the first atomic element of s
(set 'ff
    '(lambda (s)
	(cond ((atom s) s) (t (ff (car s))))))

; substitute x for all occurrences of the atom y in z
(set 'subst
    '(lambda (x y z)
	(cond ((atom z) (cond ((eq z y) x) (t z)))
	    (t (cons (subst x y (car z)) (subst x y (cdr z)))))))

; compare two s-expressions for equality
(set 'equal
    '(lambda (x y)
	(or (and (atom x) (atom y) (eq x y))
	    (and (not (atom x))
		(not (atom y))
		(equal (car x) (car y))
		(equal (cdr x) (cdr y))))))

; create a new list containing the elements of x and y
(set 'append
    '(lambda (x y)
	(cond ((null x) y)
	    (t (cons (car x) (append (cdr x) y))))))

; McCarthy's `among' function (returns t or nil)
(set 'member
    '(lambda (x y)
	(and (not (null y))
	    (or (equal x (car y)) (member x (cdr y))))))

; pair the corresponding elements of two lists
(set 'pair
    '(lambda (x y)
	(cond ((and (null x) (null y)) nil)
	    ((and (not (atom x)) (not (atom y)))
		(cons (list (car x) (car y))
		    (pair (cdr x) (cdr y)))))))

; association list selector function
; y is a list of the form ((u1 v1) ... (uN vN))
; x is one of the u's (an atom)
; here we return the pair (u v) as in most modern Lisps
(set 'assoc
    '(lambda (x y)
	(cond ((null y) nil)
	    ((eq caar y x) (car y))
	    (t (assoc x (cdr y))))))

; x is an association list
; (sublis x y) substitutes the values in x for the keys in y
(set 'sublis
    '(lambda (x y)
	(cond ((atom y) (_sublis x y))
	    (t (cons (sublis x (car y)) (sublis x (cdr y)))))))

(set '_sublis
    '(lambda (x z)
	(cond ((null x) z)
	    ((eq (caar x) z) (cadar x))
	    (t (_sublis (cdr x) z)))))

; return the last element of list e
(set 'last
    '(lambda (e)
	(cond ((atom e) nil)
	    ((null (cdr e)) (car e))
	    (t (last (cdr e))))))

; reverse a list
(set 'reverse '(lambda (x) (_reverse x nil)))
(set '_reverse
    '(lambda (x y)
	(cond ((null x) y)
	    (t (_reverse (cdr x) (cons (car x) y))))))

; remove an element from a list
(set 'remove
    '(lambda (e l)
	(cond ((null l) nil)
	    ((equal e (car l)) (remove e (cdr l)))
	    (t (cons (car l) (remove e (cdr l)))))))

; find the successor of the atom x in y
(set 'succ
    '(lambda (x y)
	(cond ((or (null y) (null (cdr y))) nil)
	    ((eq (car y) x) (cadr y))
	    (t (succ x (cdr y))))))

; find the predecessor of the atom x in y
(set 'pred
    '(lambda (x y)
	(cond ((or (null y) (null (cdr y))) nil)
	    ((eq (cadr y) x) (car y))
	    (t (pred x (cdr y))))))

; return the elements in x up to y
(set 'before
    '(lambda (x y)
	(cond ((atom x) nil)
	    ((null (cdr x)) nil)
	    ((equal (car x) y) nil)
	    ((equal (cadr x) y) (cons (car x) nil))
	    (t (cons (car x) (before (cdr x) y))))))

; return the elements in x after y
(set 'after
    '(lambda (x y)
	(cond ((atom x) nil)
	    ((equal (car x) y) (cdr x))
	    (t (after (cdr x) y)))))

; return the property list of atom x
(set 'plist '(lambda (x) (succ x Properties)))

; get the value stored on x's property list under i
(set 'get
    '(lambda (x i)
	((lambda (pr)
	    (cond ((null pr) nil)
		(t (cadr pr)))) (assoc i (plist x)))))

; store v on x's property list under i
(set 'putprop
    '(lambda (x v i)
	(and (or (plist x)
		; add a slot
		(set 'Properties (cons x (cons nil Properties))))
	    (and (set 'Properties
		    (append (before Properties x)
			(append
			    (list x
				(cons (list i v)    ; new plist
				    ((lambda (l)
					(remove (assoc i l) l)) (plist x))))
			    (cdr (after Properties x))))) v))))

; remove a property and value from x's plist
(set 'remprop
    '(lambda (x i)
	(and (get x i)	; if property exists
	    (set 'Properties
		(append (before Properties x)
		    (append
			(list x
			    ((lambda (l)
				(remove (assoc i l) l)) (plist x)))
			(cdr (after Properties x))))) i)))

; map f onto each element of l, return the list of results
(set 'mapcar
    '(lambda (f l)
	(cond ((null l) nil)
	    (t (cons (eval (list f (list 'quote (car l))))
		(mapcar f (cdr l)))))))

; call f with args, i.e., (apply 'cons '(a (b))) <-> (cons 'a '(b))
(set 'apply
    '(lambda (f args)
	(cond ((null args) nil)
	(t (eval (cons f (mapcar '(lambda (a) (list 'quote a)) args)))))))
