# batch test of sorting routines

BEGIN {
    print "    0 elements"
    isort(A, 0); check(A, 0)    
    print "    1 element"
    genid(A, 1); isort(A, 1); check(A, 1)
    
    n = 10
    print "    " n " random integers"
    genrand(A, n); isort(A, n); check(A, n)
    
    print "    " n " sorted integers"
    gensort(A, n); isort(A, n); check(A, n)
    
    print "    " n " reverse-sorted integers"
    genrev(A, n); isort(A, n); check(A, n)
    
    print "    " n " identical integers"
    genid(A, n); isort(A, n); check(A, n)
}

function isort(A,n,     i,j,t) {
    for (i = 2; i <= n; i++)
        for (j = i; j > 1 && A[j-1] > A[j]; j--) {
            # swap A[j-1] and A[j]
            t = A[j-1]; A[j-1] = A[j]; A[j] = t
        }
}

# test-generation and sorting routines...

function check(A,n,   i) {
    for (i = 1; i < n; i++)
        if (A[i] > A[i+1])
            printf("array is not sorted, element %d\n", i)
}

function genrand(A,n,  i) { # put n random integers in A
    for (i = 1; i <= n; i++)
        A[i] = int(n*rand())
}

function gensort(A,n,  i) { # put n sorted integers in A
    for (i = 1; i <= n; i++)
        A[i] = i
}

function genrev(A,n,  i) {  # put n reverse-sorted integers
    for (i = 1; i <= n; i++)  # in A
        A[i] = n+1-i
}

function genid(A,n,  i) {   # put n identical integers in A
    for (i = 1; i <= n; i++)
        A[i] = 1
}
