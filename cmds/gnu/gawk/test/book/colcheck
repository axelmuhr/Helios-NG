# colcheck - check consistency of columns
#   input:  rows of numbers and strings
#   output: lines whose format differs from first line

NR == 1	{
    nfld = NF
    for (i = 1; i <= NF; i++)
       type[i] = isnum($i)
}
{   if (NF != nfld)
       printf("line %d has %d fields instead of %d\n",
          NR, NF, nfld)
    for (i = 1; i <= NF; i++)
       if (isnum($i) != type[i])
          printf("field %d in line %d differs from line 1\n",
             i, NR)
}

function isnum(n) { return n ~ /^[+-]?[0-9]+$/ }
