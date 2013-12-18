ncsrc = [ ]

nccomp = [ nc:tripos/xcompiler/nc ]

nnc = [nc:1.6/]
ncdefs =  [-dTARGET_IS_68000 -dCOMPILING_ON_TRIPOS -dHELIOS ]
incfiles = [-i,<nnc>,nc:helios/compiler/]

ncc = [ echo "Compiling %1"
failat 1
<nccomp> -ps1 -fx <incfiles> ]

bind.tr : <nnc>BIND.C <ncsrc>
[ <ncc> <ncdefs> %1 % ]

builtin.tr : <nnc>builtin.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

cg.tr : <nnc>cg.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

codebuf.tr : <nnc>codebuf.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

driver.tr : <nnc>driver.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

flowgraf.tr : <nnc>flowgraf.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

headers.tr : <nnc>headers.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

IEEEflt.tr : <nnc>IEEEflt.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

jopprint.tr : <nnc>jopprint.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

lex.tr : <nnc>lex.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

loopopt.tr : <nnc>loopopt.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

misc.tr : <nnc>misc.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

pp.tr : <nnc>pp.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

regalloc.tr : <nnc>regalloc.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

sem.tr : <nnc>sem.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

simplify.tr : <nnc>simplify.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

syn.tr : <nnc>syn.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

synprint.tr : <nnc>synprint.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

vargen.tr : <nnc>vargen.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

m68asm.tr : <nnc>m68asm.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

m68gen.tr : <nnc>m68gen.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]

heliosobj.tr : nc:helios/compiler/heliosobj.c <ncsrc>
[
 echo "Compiling %1"
   failat 1
   <nccomp> -ps1 -fx -i,<nnc>,nc:helios/compiler/ <ncdefs>  %1 % ]

decins.tr : <nnc>decins.c <ncsrc>
[
  <ncc> <ncdefs> %1 % ]
