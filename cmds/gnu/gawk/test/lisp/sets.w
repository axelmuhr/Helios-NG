; set operations on lists

(set 'union
    '(lambda (x y)
	(cond ((null x) y)
	    ((member (car x) y) (union (cdr x) y))
	    (t (cons (car x) (union (cdr x) y))))))

(set 'intersection
    '(lambda (x y)
	(cond ((null x) nil)
	    ((member (car x) y)
		(cons (car x) (intersection (cdr x) y)))
	    (t (intersection (cdr x) y)))))

(set 'ldifference
    '(lambda (in out)
	(cond ((null in) nil)
	    ((member (car in) out) (ldifference (cdr in) out))
	    (t (cons (car in) (ldifference (cdr in) out))))))

(set 'subsetp
    '(lambda (a b)
	(cond ((null a) t)
	    ((member (car a) b) (subsetp (cdr a) b))
	    (t nil))))

(set 'samesetp
    '(lambda (a b)
	(and (subsetp a b) (subsetp b a))))
