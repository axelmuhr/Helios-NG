# heapsort

    { A[NR] = $0 }

END { hsort(A, NR)
      for (i = 1; i <= NR; i++)
          { print A[i] }
    }

function hsort(A,n,  i) {
    for (i = int(n/2); i >= 1; i--)  # phase 1
         { heapify(A, i, n) }
    for (i = n; i > 1; i--) {        # phase 2
         { swap(A, 1, i) }
         { heapify(A, 1, i-1) }
    }
}
function heapify(A,left,right,   p,c) {
    for (p = left; (c = 2*p) <= right; p = c) {
        if (c < right && A[c+1] > A[c])
            { c++ }
        if (A[p] < A[c])
            { swap(A, c, p) }
    }
}
function swap(A,i,j,   t) {
    t = A[i]; A[i] = A[j]; A[j] = t
}
