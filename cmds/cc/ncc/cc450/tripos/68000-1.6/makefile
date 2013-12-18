ncsrc = [ ]

nccomp = [ c:nc ]

ncdefs =  [-dTARGET_IS_68000 -dCOMPILING_ON_AMIGA -dSTATIC_32BIT_CALLS ]

nnc = [/1.6/]

ncc = [ echo "Compiling %1" 
   <nccomp> -ps1 -fx -i:chris/cc<nnc> <ncdefs>]

nc : bind.o builtin.o cg.o codebuf.o
   driver.o flowgraf.o headers.o IEEEflt.o jopprint.o
   lex.o loopopt.o misc.o pp.o regalloc.o
   sem.o simplify.o syn.o synprint.o vargen.o
   m68asm.o m68gen.o m68obj.o decins.o nc:nclib/clib
[ blink with nclink to nc library nc:1.6lib/clib+lib:amiga.lib map linkmap xref]

bind.o : <nnc>BIND.C <ncsrc>
[ <ncc> %1 % ]

builtin.o : <nnc>builtin.c <ncsrc>
[
  <ncc> %1 % ]

cg.o : <nnc>cg.c <ncsrc>
[
  <ncc> %1 % ]

codebuf.o : <nnc>codebuf.c <ncsrc>
[
  <ncc> %1 % ]

driver.o : <nnc>driver.c <ncsrc>
[
  <ncc> -dNO_INSTORE_FILES %1 % ]

flowgraf.o : <nnc>flowgraf.c <ncsrc>
[
  <ncc> %1 % ]

headers.o : /1.6/headers.c <ncsrc>
[
  <ncc> %1 % ]

IEEEflt.o : /norcroftsave/IEEEflt.c <ncsrc>
[
  <ncc> %1 % ]

jopprint.o : <nnc>jopprint.c <ncsrc>
[
  <ncc> %1 % ]

lex.o : <nnc>lex.c <ncsrc>
[
  <ncc> %1 % ]

loopopt.o : <nnc>loopopt.c <ncsrc>
[
  <ncc> %1 % ]

misc.o : <nnc>misc.c <ncsrc>
[
  <ncc> %1 % ]

pp.o : <nnc>pp.c <ncsrc>
[
  <ncc> %1 % ]

regalloc.o : <nnc>regalloc.c <ncsrc>
[
  <ncc> %1 % ]

sem.o : <nnc>sem.c <ncsrc>
[
  <ncc> %1 % ]

simplify.o : <nnc>simplify.c <ncsrc>
[
  <ncc> %1 % ]

syn.o : <nnc>syn.c <ncsrc>
[
  <ncc> %1 % ]

synprint.o : <nnc>synprint.c <ncsrc>
[
  <ncc> %1 % ]

vargen.o : <nnc>vargen.c <ncsrc>
[
  <ncc> %1 % ]

m68asm.o : <nnc>m68asm.c <ncsrc>
[
  <ncc> %1 % ]

m68gen.o : /nc68020/m68gen.c <ncsrc>
[
  <ncc> %1 % ]

m68obj.o : <nnc>m68obj.c <ncsrc>
[
  <ncc> %1 % ]

decins.o : <nnc>decins.c <ncsrc>
[
  <ncc> %1 % ]
