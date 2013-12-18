BEGIN { srand(1111) }
{ print }
# interactive test framework for sort routines

/^[0-9]+.*rand/ { n = $1; genrand(A, n); dump(A, n); next }
/^[0-9]+.*id/   { n = $1; genid(A, n); dump(A, n); next }
/^[0-9]+.*sort/ { n = $1; gensort(A, n); dump(A, n); next }
/^[0-9]+.*rev/  { n = $1; genrev(A, n); dump(A, n); next }
/^data/ {   # use data right from this line
	for (i = 2; i <= NF; i++)
		A[i-1] = $i
	n = NF - 1
	next
}
/q.*sort/ { qsort(A, 1, n); check(A, n); dump(A, n); next }
/h.*sort/ { hsort(A, n); check(A, n); dump(A, n); next }
/i.*sort/ { isort(A, n); check(A, n); dump(A, n); next }
/./  { print "data ... | N [rand|id|sort|rev]; [qhi]sort" }

function dump(A, n) {    # print A[1]..A[n]
	for (i = 1; i <= n; i++)
		printf(" %s", A[i])
	printf("\n")
}

# test-generation and sorting routines ...

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
          for (j = i; j > 1 && A[j-1] > A[j]; j--) {
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
         if (A[i] < A[left])
             swap(A, ++last, i)
     swap(A, left, last)
     qsort(A, left, last-1)
     qsort(A, last+1, right)
}

function swap(A,i,j,   t) {
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
         if (c < right && A[c+1] > A[c])
             c++
         if (A[p] < A[c])
             swap(A, c, p)
     }
}
