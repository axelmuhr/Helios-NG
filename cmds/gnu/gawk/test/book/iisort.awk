# count comparisons and exchanges in isort

    { A[NR] = $0 }
END { comp = exch = 0
      isort(A, NR)
      print "isort", NR, comp, exch
    }

function isort(A,n,     i,j,t) {  # insertion sort
    for (i = 2; i <= n; i++)      # with counters
        for (j = i; j > 1 && ++comp &&
          A[j-1] > A[j] && ++exch; j--) {
            # swap A[j-1] and A[j]
            t = A[j-1]; A[j-1] = A[j]; A[j] = t
        }
}
