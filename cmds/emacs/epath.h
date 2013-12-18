/*	PATH:	This file contains certain info needed to locate the
		MicroEMACS files on a system dependant basis.

	$Header: /hsrc/cmds/emacs/RCS/epath.h,v 1.1 1990/08/23 15:14:11 jon Exp $

									*/

/*	possible names and paths of help files under different OSs	*/

char *pathname[] =

#if	AMIGA
{
	".emacsrc",
	"emacs.hlp",
	"",
	":c/",
	":t/"
};
#endif

#if	ST520
{
	"emacs.rc",
	"emacs.hlp",
	"\\",
	"\\bin\\",
	"\\util\\",
	""
};
#endif

#if	HELIOS
{
	"emacs.rc",
	"emacs.hlp",
	"",
	"/helios/bin/",
	"/helios/etc/"
};
#endif

#if	FINDER
{
	"emacs.rc",
	"emacs.hlp",
	"/bin",
	"/sys/public",
	""
};
#endif

#if	MSDOS
{
	"emacs.rc",
	"emacs.hlp",
	"\\sys\\public\\",
	"\\usr\\bin\\",
	"\\bin\\",
	"\\",
	""
};
#endif

#if	V7 | BSD | USG
{
	".emacsrc",
	"emacs.hlp",
	"/usr/local/",
	"/usr/lib/",
	""
};
#endif

#if	VMS
{
	"emacs.rc",
	"emacs.hlp",
	"",
	"sys$sysdevice:[vmstools]"
};
#endif

#define	NPNAMES	(sizeof(pathname)/sizeof(char *))
