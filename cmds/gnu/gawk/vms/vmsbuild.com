$! vmsbuild.com -- Commands to build GAWK		Pat Rankin, Dec'89
$!							   revised, Mar'90
$!						gawk 2.13  revised, Jun'91
$!
$ REL = "2.13"	!release version number
$ PATCHLVL = "2"
$!
$!	[ remove "/optimize=noinline" for VAX C V2.x ]
$ if f$type(cc)  .nes."STRING" then  cc   := cc/nolist/optimize=noinline
$ if f$type(link).nes."STRING" then  link := link/nomap
$!
$ cc := 'cc'/include=[]
$ libs = "sys$share:vaxcrtl.exe/Shareable"
$
$! uncomment the next two lines for VAX C V2.x
$ ! define vaxc$library sys$library:,sys$disk:[.vms]
$ ! define c$library [],[.vms]
$!
$! uncomment next two lines for GNU C
$ ! cc := gcc/include=([],[.vms])	!use GNU C rather than VAX C
$ ! libs = "gnu_cc:[000000]gcclib.olb/Library,sys$library:vaxcrtl.olb/Library"
$!
$ if f$search("config.h") .eqs."" then  copy [.config]vms-conf.h []config.h
$ if f$search("awk_tab.c").nes."" then  goto awk_tab_ok
$	write sys$output " You must process `awk.y' with ""yacc"" or ""bison"""
$	if f$search("awk.tab_c").nes."" then -	!unpacked with poor 'tar' reader
		write sys$output " or else rename `awk.tab_c' to `awk_tab.c'."
$	if f$search("y_tab.c").nes."" then -	!yacc was run manually
		write sys$output " or else rename `y_tab.c' to `awk_tab.c'."
$	exit
$awk_tab_ok:
$ cc main.c
$ cc eval.c
$ cc builtin.c
$ cc msg.c
$ cc iop.c
$ cc io.c
$ cc field.c
$ cc array.c
$ cc node.c
$ cc version.c
$ cc missing.c
$ cc awk_tab.c
$ cc regex.c
$ cc re.c
$ cc dfa.c
$ cc/define=("STACK_DIRECTION=(-1)","exit=vms_exit") alloca
$ cc [.vms]vms_misc.c
$ cc [.vms]vms_popen.c
$ cc [.vms]vms_fwrite.c
$ cc [.vms]vms_args.c
$ cc [.vms]vms_gawk.c
$ cc [.vms]vms_cli.c
$ set command/object=[]gawk_cmd.obj [.vms]gawk.cld
$!
$ create gawk.opt
! GAWK -- Gnu AWK
main.obj,eval.obj,builtin.obj,msg.obj,iop.obj,io.obj
field.obj,array.obj,node.obj,version.obj,missing.obj,awk_tab.obj
regex.obj,re.obj,dfa.obj,[]alloca.obj
[]vms_misc.obj,vms_popen.obj,vms_fwrite.obj
[]vms_args.obj,vms_gawk.obj,vms_cli.obj,gawk_cmd.obj
psect_attr=environ,noshr	!extern [noshare] char **
stack=50	!preallocate more pages (default is 20)
$ open/append Fopt gawk.opt
$ write Fopt libs
$ write Fopt "identification=""V''REL'.''PATCHLVL'"""
$ close Fopt
$!
$ link/exe=gawk.exe gawk.opt/options
