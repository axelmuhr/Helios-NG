# insertion sort

    { A[NR] = $0 }

END { isort(A, NR)
      for (i = 1; i <= NR; i++)
          print A[i]
    }

# isort - sort A[1..n] by insertion

function isort(A,n,     i,j,t) {
    for (i = 2; i <= n; i++)
        for (j = i; j > 1 && A[j-1] > A[j]; j--) {
            # swap A[j-1] and A[j]
            t = A[j-1]; A[j-1] = A[j]; A[j] = t
        }
}
