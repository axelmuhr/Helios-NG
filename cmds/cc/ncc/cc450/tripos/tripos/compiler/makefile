ncsrc = [ ]

nccomp = [/xcompiler/nc]

ncdefs =  [-dTARGET_IS_68000 -dTRIPOS -dSTATIC_32BIT_CALLS]

nnc = [nc:1.6/]

ncc = [ echo "Compiling %1" 
<nccomp> -ps1 -fx -i,<nnc> <ncdefs>]

bind.tr : <nnc>BIND.C <ncsrc>
[ <ncc>  %1 % ]

builtin.tr : <nnc>builtin.c <ncsrc>
[
  <ncc>  %1 % ]

cg.tr : <nnc>cg.c <ncsrc>
[
  <ncc>  %1 % ]

codebuf.tr : <nnc>codebuf.c <ncsrc>
[
  <ncc>  %1 % ]

driver.tr : <nnc>driver.c <ncsrc>
[
  <ncc>  -dNO_INSTORE_FILES %1 % ]

flowgraf.tr : <nnc>flowgraf.c <ncsrc>
[
  <ncc>  %1 % ]

headers.tr : <nnc>headers.c <ncsrc>
[
  <ncc>  %1 % ]

IEEEflt.tr : <nnc>IEEEflt.c <ncsrc>
[
  <ncc>  %1 % ]

jopprint.tr : <nnc>jopprint.c <ncsrc>
[
  <ncc>  %1 % ]

lex.tr : <nnc>lex.c <ncsrc>
[
  <ncc>  %1 % ]

loopopt.tr : <nnc>loopopt.c <ncsrc>
[
  <ncc>  %1 % ]

misc.tr : <nnc>misc.c <ncsrc>
[
  <ncc>  %1 % ]

pp.tr : <nnc>pp.c <ncsrc>
[
  <ncc>  %1 % ]

regalloc.tr : <nnc>regalloc.c <ncsrc>
[
  <ncc>  %1 % ]

sem.tr : <nnc>sem.c <ncsrc>
[
  <ncc>  %1 % ]

simplify.tr : <nnc>simplify.c <ncsrc>
[
  <ncc>  %1 % ]

syn.tr : <nnc>syn.c <ncsrc>
[
  <ncc>  %1 % ]

synprint.tr : <nnc>synprint.c <ncsrc>
[
  <ncc>  %1 % ]

vargen.tr : <nnc>vargen.c <ncsrc>
[
  <ncc>  %1 % ]

m68asm.tr : <nnc>m68asm.c <ncsrc>
[
  <ncc>  %1 % ]

triposgen.tr : <nnc>m68gen.c <ncsrc>
[
  <ncc> %1 % ]

triposobj.tr : <nnc>triposobj.c <ncsrc>
[
  <ncc> %1 % ]

decins.tr : <nnc>decins.c <ncsrc>
[
  <ncc> %1 % ]
