ncsrc = [ ]

nccomp = [ c:nc ]

ncdefs =  [-dTARGET_IS_68000 -dCOMPILING_ON_AMIGA -dHELIOS ]

nnc = [nc:1.6/]

ncc = [ echo "Compiling %1"
   failat 1
   <nccomp> -ps1 -fx -i,<nnc> <ncdefs>]

nc : bind.o builtin.o cg.o codebuf.o
   driver.o flowgraf.o headers.o IEEEflt.o jopprint.o
   lex.o loopopt.o misc.o pp.o regalloc.o
   sem.o simplify.o syn.o synprint.o vargen.o
   m68asm.o m68gen.o heliosobj.o decins.o nc:lib/clib nc:lib/ncamiga.lib
[ alink with nclink to nc]

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
  <ncc> %1 % ]

flowgraf.o : <nnc>flowgraf.c <ncsrc>
[
  <ncc> %1 % ]

headers.o : <nnc>headers.c <ncsrc>
[
  <ncc> %1 % ]

IEEEflt.o : <nnc>IEEEflt.c <ncsrc>
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

m68gen.o : <nnc>m68gen.c <ncsrc>
[
  <ncc> %1 % ]

heliosobj.o : heliosobj.c <ncsrc>
[
  <ncc> %1 % ]

decins.o : <nnc>decins.c <ncsrc>
[
  <ncc> %1 % ]
