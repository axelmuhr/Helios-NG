ncsrc = [  ]
nccomp = [ /compiler/nc ]
oml = [ oml clib r % ]
nnc = [ nc:1.6lib/]


nc = [ echo "Compiling %" 
failat 1
<nccomp> -dAMIGA -dTARGET_IS_68020 -ps1 -i,nc:1.6lib/,include: ]

scanf.o      : <nnc>SCANF.C      <ncsrc> [<nc> %1 %
]

