# makeprof - prepare profiling version of an awk program
#   usage:  awk -f makeprof awkprog >awkprog.p
#   running awk -f awkprog.p data creates a
#       file prof.cnts of statement counts for awkprog

    { if ($0 ~ /{/) sub(/{/, "{ _LBcnt[" ++_numLB "]++; ")
      print
    }

END { printf("END { for (i = 1; i <= %d; i++)\n", _numLB)
      printf("\t\t print _LBcnt[i] > \"prof.cnts\"\n}\n")
    }
