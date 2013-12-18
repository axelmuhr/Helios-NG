# quicksort

    { A[NR] = $0 }

END { qsort(A, 1, NR)
      for (i = 1; i <= NR; i++)
          print A[i]
    }

# qsort - sort A[left..right] by quicksort

function qsort(A,left,right,   i,last) {
     if (left >= right)  # do nothing if array contains
         return          # less than two elements
     swap(A, left, left + int((right-left+1)*rand()))
     last = left   # A[left] is now partition element
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
