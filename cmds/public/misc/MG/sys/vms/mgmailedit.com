$	Verify = 'F$Verify(0)
$ ! Command procedure to invoke MG from MAIL.  You should
$ ! have the symbol MG globally defined, either as
$ !
$ !	MG :== $dev:[dir]MG
$ !
$ ! Then
$ !	DEFINE MAIL$EDIT dev:[dir]MGMAILEDIT.COM
$ !
$ ! to make MAIL look for this file.
$ !
$ ! or, if using the kept-fork capability,
$ !
$ !	MG :== @dev:[dir]MG.COM
$ !
$ ! Inputs:
$ !
$ !	P1 = Input file name.
$ !	P2 = Output file name.
$ !
$ ! The default directory is the same as the parent process.
$ !
$ ! Copy the input file to the output file, then invoke MG on it.
$ !
$ Set Noon
$ Define/Job MG$AttachTo "''F$Process()'"
$ If P2 .Nes. "" .AND. P1 .Nes. "" Then Copy 'P1' 'P2'
$ Define/User Sys$Input Sys$Command
$ MG 'P2'
$ If F$Trnlnm("MG$AttachTo") .Nes. "" Then - ! MG.COM might have done it already
	Deassign/Job MG$AttachTo
$ If Verify Then -
	Set Verify
