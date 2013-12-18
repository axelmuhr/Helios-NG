ncsrc = [  ]
nclib = [nc:1.6lib/]
nccomp = [ /compiler/nc ]
oml = [ oml clib r % ]

nc = [ echo "Compiling %" 
failat 1
<nccomp> -dNEWTRIPOS -dTARGET_IS_68000 -ps1 -i,/headers/,nc:1.6lib/ ]

asm = [ echo "Assembling %1"
        failat 1
        assem %1 -o % -i include: -h sysdef.i
]

stdio.o      : <nclib>stdio.c      <ncsrc> [<nc> %1 %]
string.o     : <nclib>string.c     <ncsrc> [<nc> %1 %]
alloc.o      : <nclib>alloc.c      <ncsrc> [<nc> %1 %]
stdlib.o     : <nclib>stdlib.c     <ncsrc> [<nc> %1 %]
signal.o     : <nclib>signal.c     <ncsrc> [<nc> %1 %]
printf.o     : <nclib>printf.c     <ncsrc> [<nc> %1 %]
ctype.o      : <nclib>ctype.c      <ncsrc> [<nc> %1 %]
error.o      : <nclib>error.c      <ncsrc> [<nc> %1 %]
mc68000sys.o : <nclib>mc68000sys.c <ncsrc> [<nc> %1 %]
time.o       : <nclib>time.c       <ncsrc> [<nc> %1 %]
softfp.o     : <nclib>softfp.c     <ncsrc> [<nc> -dIEEE %1 %]
locale.o     : <nclib>locale.c     <ncsrc> [<nc> %1 %]
sort.o       : <nclib>SORT.C       <ncsrc> [<nc> %1 %]
math.o       : <nclib>MATH.C       <ncsrc> [<nc> %1 %]
scanf.o      : <nclib>SCANF.C      <ncsrc> [<nc> %1 %]
fpprintf.o   : <nclib>FPPRINTF.C   <ncsrc> [<nc> %1 %]

arith.o      : <nclib>arith.a      [<asm>]
c.o          : <nclib>c.a          [<asm>]
setjmp.o     : <nclib>setjmp.a     [<asm>]

dos.o        : dos.a               [<asm>]
kernel.o     : kernel.a            [<asm>]
