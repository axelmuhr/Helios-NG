# printprof - print profiling counts
#     usage:  awk -f printprof awkprog
#     prints awkprog with statement counts from prof.cnts

BEGIN { while (getline < "prof.cnts" > 0) cnt[++i] = $1 }
/{/   { printf("%5d", cnt[++j]) }
      { printf("\t%s\n", $0) }
