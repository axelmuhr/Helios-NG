# test framework for sort performance evaluation
#   input:  lines with sort name, type of data, sizes...
#   output: name, type, size, comparisons, exchanges, c+e

{   for (i = 3; i <= NF; i++)
        test($1, $2, $i)
}

function test(sort, data, n) {
    comp = exch = 0
    if (data ~ /rand/)
        genrand(A, n)
    else if (data ~ /id/)
        genid(A, n)
    else if (data ~ /rev/)
        genrev(A, n)
    else
        print "illegal type of data in", $0
    if (sort ~ /q.*sort/)
        qsort(A, 1, n)
    else if (sort ~ /h.*sort/)
        hsort(A, n)
    else if (sort ~ /i.*sort/)
        isort(A, n)
    else print "illegal type of sort in", $0
    print sort, data, n, comp, exch, comp+exch
}

# test-generation and sorting routines ...

BEGIN { srand(111) }
function genrand(A,n,    i) { # put n random integers in A
    for (i = 1; i <= n; i++)
        A[i] = int(n*rand())
}

function gensort(A,n,    i) { # put n sorted integers in A
    for (i = 1; i <= n; i++)
        A[i] = i
}

function genrev(A,n,    i) { # put n reverse-sorted integers in A
    for (i = 1; i <= n; i++)
        A[i] = n+1-i
}

function genid(A,n,    i) { # put n identical integers in A
    for (i = 1; i <= n; i++)
        A[i] = 1
}

function check(A,n,   i) {
    for (i = 1; i < n; i++)
        if (A[i] > A[i+1])
            printf("error: array is not sorted, element %d\n", i)
}
function isort(A,n,        i,j,t) {
      for (i = 2; i <= n; i++)
          for (j = i; j > 1 && ++comp && A[j-1] > A[j] && ++exch; j--) {
              # swap A[j-1] and A[j]
              t = A[j-1]; A[j-1] = A[j]; A[j] = t
          }
}

function qsort(A,left,right,   i,last) {
     if (left >= right)  # do nothing if array contains
         return          # at most one element
     swap(A, left, left + int((right-left+1)*rand()))
     last = left
     for (i = left+1; i <= right; i++)
         if (++comp && A[i] < A[left])
             swap(A, ++last, i)
     swap(A, left, last)
     qsort(A, left, last-1)
     qsort(A, last+1, right)
}

function swap(A,i,j,   t) {
     ++exch
     t = A[i]; A[i] = A[j]; A[j] = t
}

function hsort(A,right,  i) {
     for (i = int(right/2); i >= 1; i--)
          heapify(A, i, right)
     for (i = right; i > 1; i--) {
          swap(A, 1, i)
          heapify(A, 1, i-1)
     }
}

function heapify(A,left,right,   p,c) {
     for (p = left; (c = 2*p) <= right; p = c) {
         if (c < right && ++comp && A[c+1] > A[c])
             c++
         if (++comp && A[p] < A[c])
             swap(A, c, p)
     }
}
