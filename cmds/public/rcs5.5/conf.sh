#!/bin/sh
# Output RCS compile-time configuration.
Id='$Id: conf.sh,v 5.7 91/01/09 15:03:47 chris Exp $'
#	Copyright 1990 by Paul Eggert
#	Distributed under license by the Free Software Foundation, Inc.

# This file is part of RCS.
#
# RCS is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 1, or (at your option)
# any later version.
#
# RCS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with RCS; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
#
# Report problems and direct all questions to:
#
#     rcs-bugs@cs.purdue.edu


# Direct standard output to "a.h"; later parts of this procedure need it.
# Standard error can be ignored if a.h is OK,
# and can be inspected for clues otherwise.
exec >a.h || exit

# The Makefile overrides the following defaults.
: ${C='cc -O'}
: ${COMPAT2=0}
: ${DIFF_FLAGS=-an}
: ${DIFF_L=1}
: ${RCSPREFIX=/usr/local/bin/}
: ${SENDMAIL=/usr/lib/sendmail}
: ${L=}
: ${DIFF=${RCSPREFIX}diff}


cat <<EOF || exit
/* RCS compile-time configuration */

	/* $Id */

/*
 * This file is generated automatically.
 * If you edit it by hand your changes may be lost.
 * Instead, please try to fix conf.sh,
 * and send your fixes to rcs-bugs@cs.purdue.edu.
 */

EOF

: exitmain
cat >a.c <<EOF && $C a.c $L >&2 || exit
#include "a.h"
int main(argc,argv) int argc; char **argv; { return argc-1; }
EOF
e='(n) ? (exit(n),(n)) : (n) /* lint fodder */'
if ./a.out -
then :
elif ./a.out
then e=n
fi
echo "#define exitmain(n) return $e /* how to exit from main() */"

: standard includes
standardIncludes=
for h in stdio sys/types sys/stat fcntl limits stdlib string unistd vfork
do
	cat >a.c <<EOF || exit
#include "a.h"
$standardIncludes
#include <$h.h>
int main(){ exitmain(0); }
EOF
	if ($C a.c $L && ./a.out) >&2
	then i="#	include <$h.h>"
	else
		case $h in
		string)
			i='#	include <strings.h>';;
		*)
			i="	/* #include <$h.h> does not work.  */"
		esac
	fi || exit
	standardIncludes="$standardIncludes
$i"
done

cat <<EOF || exit
#if !MAKEDEPEND$standardIncludes
#endif /* !MAKEDEPEND */
EOF

# has_sys_*_h
for H in dir param wait
do
	: has_sys_${H}_h
	cat >a.c <<EOF || exit
#include "a.h"
#include <sys/$H.h>
int main() { exitmain(0); }
EOF
	if ($C a.c $L && ./a.out) >&2
	then h=1
	else h=0
	fi
	echo "#define has_sys_${H}_h $h /* Does #include <sys/$H.h> work?  */"
done

: const, volatile
for i in const volatile
do
	cat >a.c <<EOF || exit
#	include "a.h"
	$i int * $i * zero;
EOF
	if $C -S a.c >&2
	then echo "/* #define $i */ /* The '$i' keyword works.  */"
	else echo "#define $i /* The '$i' keyword does not work.  */"
	fi
done

# *_t
cat >a.c <<'EOF' || exit
#include "a.h"
#include <signal.h>
t x;
EOF
for t in gid_t mode_t pid_t sig_atomic_t size_t time_t uid_t
do
	: $t
	case $t in
	time_t) i=long;;
	*) i=int;;
	esac
	if $C -S -Dt=$t a.c >&2
	then echo "/* typedef $i $t; */ /* Standard headers define $t.  */"
	else echo "typedef $i $t; /* Standard headers do not define $t.  */"
	fi
done

: has_prototypes
cat >a.c <<'EOF' || exit
#include "a.h"
int main(int, char**);
int main(int argc, char **argv) { exitmain(!argv[argc-1]); }
EOF
if $C -S a.c >&2
then h=1
else h=0
fi
cat <<EOF
#define has_prototypes $h /* Do function prototypes work?  */
#if has_prototypes
#	define P(params) params
#	if !MAKEDEPEND
#		include <stdarg.h>
#	endif
#	define vararg_start(ap,p) va_start(ap,p)
#else
#	define P(params) ()
#	if !MAKEDEPEND
#		include <varargs.h>
#	endif
#	define vararg_start(ap,p) va_start(ap)
#endif
EOF

: has_getuid
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef getuid
	uid_t getuid();
#endif
int main() { exitmain(getuid()!=getuid()); }
EOF
if ($C a.c $L && ./a.out) >&2
then h=1
else h=0
fi
echo "#define has_getuid $h /* Does getuid() work?  */"

: declare_getpwuid
cat >a.c <<'EOF' || exit
#include "a.h"
#include <pwd.h>
d
int main() { exitmain(!getpwuid(0)); }
EOF
D='struct passwd *getpwuid P((uid_t));'
if ($C -Dd="$D" a.c $L && ./a.out) >&2
then define="#define declare_getpwuid $D"
elif ($C -Dd= a.c $L && ./a.out) >&2
then define="#define declare_getpwuid /* $D */"
else define="/* #define declare_getpwuid $D */"
fi
echo "$define"

: has_rename, bad_rename
cat >a.c <<'EOF' && rm -f a.d || exit
#include "a.h"
#ifndef rename
	int rename();
#endif
int main() { exitmain(rename("a.c","a.d")); }
EOF
if ($C a.c $L && ./a.out && test -f a.d) >&2
then
	h=1
	echo x >a.c
	if ./a.out && test ! -f a.c -a -f a.d
	then b=0
	else b=1
	fi
else h=0 b=0
fi
echo "#define has_rename $h /* Does rename() work?  */"
echo "#define bad_rename $b /* Does rename(A,B) fail if B exists?  */"

: void, VOID
cat >a.c <<'EOF' || exit
#include "a.h"
void f() {}
int main() {f(); exitmain(0);}
EOF
if $C -S a.c >&2
then
	v='(void) '
else
	v=
	echo 'typedef int void;'
fi
echo "#define VOID $v/* 'VOID e;' discards the value of an expression 'e'.  */"

: signal_type, sig_zaps_handler
cat >a.c <<'EOF' || exit
#include "a.h"
#include <signal.h>
#ifndef getpid
	pid_t getpid();
#endif
#ifndef kill
	int kill();
#endif
#ifndef signal
	signal_type (*signal P((int,signal_type(*)P((int)))))P((int));
#endif
signal_type nothing(i) int i; {}
int main(argc, argv) int argc; char **argv;
{
	signal(SIGINT, nothing);
	while (--argc)
		kill(getpid(), SIGINT);
	exitmain(0);
}
EOF
for signal_type in void int bad
do
	case $signal_type in
	bad) echo >&2 "cannot deduce signal_type"; exit 1
	esac
	($C -Dsignal_type=$signal_type a.c $L && ./a.out 1) >&2 && break
done
echo "#define signal_type $signal_type /* type returned by signal handlers */"
if ./a.out 1 2 >&2
then z=0
else z=1
fi
echo "#define sig_zaps_handler $z /* Must a signal handler reinvoke signal()?  */"

: has_seteuid
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef geteuid
	uid_t geteuid();
#endif
int main() {
/* Guess, don't test.  Ugh.  Testing would require running conf.sh setuid.  */
#if !_POSIX_VERSION || _POSIX_VERSION==198808L&&!defined(sun)&&!defined(__sun__)
	exitmain(1);
#else
	exitmain(seteuid(geteuid()) < 0);
#endif
}
EOF
if ($C a.c $L && ./a.out) >&2
then h=1
else h=0
fi
echo "#define has_seteuid $h /* Does seteuid() obey Posix 1003.1-1990?  */"

: has_sigaction
cat >a.c <<'EOF' || exit
#include <signal.h>
struct sigaction s;
int a = sizeof(s);
EOF
if $C -S a.c >&2
then h=1
else h=0
fi
echo "#define has_sigaction $h /* Does struct sigaction work?  */"

: has_sigblock
cat >a.c <<'EOF' || exit
#include "a.h"
#include <signal.h>
#ifndef sigblock
	int sigblock();
#endif
int main() { sigblock(0); exitmain(0); }
EOF
if ($C a.c $L && ./a.out) >&2
then h=1
else h=0
fi
echo "#define has_sigblock $h /* Does sigblock() work?  */"

: has_sys_siglist
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef sys_siglist
	extern const char *sys_siglist[];
#endif
int main() { exitmain(!sys_siglist[1][0]); }
EOF
if ($C a.c $L && ./a.out) >&2
then h=1
else h=0
fi
echo "#define has_sys_siglist $h /* Does sys_siglist[] work?  */"

: exit_type, underscore_exit_type
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef exit
	void exit();
#endif
int main() { exit(0); }
EOF
if $C -S a.c >&2
then t=void
else t=int
fi
echo "#define exit_type $t /* type returned by exit() */"
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef _exit
	void _exit();
#endif
int main() { _exit(0); }
EOF
if $C -S a.c >&2
then t=void
else t=int
fi
echo "#define underscore_exit_type $t /* type returned by _exit() */"

: fread_type
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef fread
	size_t fread();
#endif
int main() { char b; exitmain(!(fread(&b,1,1,stdin)==1 && b=='#')); }
EOF
if $C -S a.c $L >&2
then t=size_t
else t=int
fi
echo "typedef $t fread_type; /* type returned by fread() and fwrite() */"

: malloc_type
cat >a.c <<'EOF' || exit
#include "a.h"
typedef void *malloc_type;
#ifndef malloc
	malloc_type malloc();
#endif
int main() { exitmain(!malloc(1)); }
EOF
if $C -S a.c >&2
then t=void
else t=char
fi
echo "typedef $t *malloc_type; /* type returned by malloc() */"

: free_type
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef malloc
	malloc_type malloc();
#endif
#ifndef free
	void free();
#endif
int main() { free(malloc(1)); exitmain(0); }
EOF
if $C -S a.c >&2
then t=void
else t=int
fi
echo "#define free_type $t /* type returned by free() */"

: strlen_type
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef strlen
	size_t strlen();
#endif
int main() { exitmain(strlen("")); }
EOF
if $C -S a.c >&2
then t=size_t
else t=int
fi
echo "typedef $t strlen_type; /* type returned by strlen() */"

: has_getcwd
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef getcwd
	char *getcwd();
#endif
char buf[10000];
int main() { exitmain(!getcwd(buf,10000)); }
EOF
if ($C a.c $L && ./a.out) >&2
then has_getcwd=1
else has_getcwd=0
fi
echo "#define has_getcwd $has_getcwd /* Does getcwd() work?  */"

: has_getwd
case $has_getcwd in
0)
	cat >a.c <<'EOF' || exit
#include "a.h"
#include <sys/param.h>
#ifndef getwd
	char *getwd();
#endif
char buf[MAXPATHLEN];
int main() { exitmain(!getwd(buf)); }
EOF
	if ($C a.c $L && ./a.out) >&2
	then h=1
	else h=0
	fi
	echo "#define has_getwd $h /* Does getwd() work?  */";;
1)
	echo "/* #define has_getwd ? */ /* Does getwd() work?  */"
esac

: has_vfork
cat >a.c <<'EOF' || exit
#include "a.h"
#if has_sys_wait_h
#	include <sys/wait.h>
#endif
#ifndef _exit
	underscore_exit_type _exit();
#endif
#ifndef getpid
	pid_t getpid();
#endif
#ifndef wait
	pid_t wait();
#endif
pid_t child;
int status;
struct stat st;
int main()
{
	pid_t parent = getpid();
	if (!(child = vfork())) {
		/* Tickle vfork/compiler bug (e.g. sparc gcc -O (1.37.1).  */
		pid_t i = getpid(), j = getpid();
		if (i!=getpid() || j!=getpid())
			_exit(!i);
		/* Tickle file descriptor bug (e.g. IRIX 3.3).  */
		_exit(close(1) < 0);
	} else {
		while (wait(&status) != child)
			;
		/* Test for presence of bugs.  */
		exitmain(status  ||  parent != getpid()  ||  fstat(1, &st) < 0);
	}
}
EOF
if ($C a.c $L && ./a.out) >&2
then h=1
else h=0
fi
echo "#define has_vfork $h /* Does vfork() work?  */"

: has_vfprintf
cat >a.c <<'EOF' || exit
#include "a.h"
#if has_prototypes
int p(const char *format,...)
#else
/*VARARGS1*/ int p(format, va_alist) char *format; va_dcl
#endif
{
	int r;
	va_list args;
	vararg_start(args, format);
	r = vfprintf(stderr, format, args);
	va_end(args);
	return r;
}
int main() { exitmain(p("")); }
EOF
if ($C a.c $L && ./a.out) >&2
then h=1
else h=0
fi
echo "#define has_vfprintf $h /* Does vfprintf() work?  */"

: strrchr
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef strrchr
	char *strrchr();
#endif
int main() {exitmain(!strrchr("abc", 'c'));}
EOF
($C a.c $L && ./a.out) >&2 ||
  echo "#define strrchr rindex /* Use old-fashioned name for strrchr(). */"

: CO
echo "#define CO \"${RCSPREFIX}co\" /* name of 'co' program */"

: COMPAT2
echo "#define COMPAT2 $COMPAT2 /* Are version 2 files supported?  */"

: DATEFORM
cat >a.c <<'EOF' || exit
#include "a.h"
int main() { printf("%.2d", 1); exitmain(0); }
EOF
$C a.c $L >&2 && r=`./a.out` || exit
case $r in
01)	f=%.2d;;
*)	f=%02d
esac
echo "#define DATEFORM \"$f.$f.$f.$f.$f.$f\" /* e.g. 01.01.01.01.01.01 */"

: DIFF
echo "#define DIFF \"$DIFF\" /* name of 'diff' program */"

: DIFF_FLAGS
dfs=
for df in $DIFF_FLAGS
do dfs="$dfs, \"$df\""
done
echo "#define DIFF_FLAGS $dfs /* Make diff output suitable for RCS.  */"

: DIFF_L
echo "#define DIFF_L $DIFF_L /* Does diff -L work? */"

: EXECRCS
e=execv
for i in "$DIFF" "$RCSPREFIX" "$SENDMAIL"
do
	case $i in
	*/*) ;;
	*) e=execvp break
	esac
done
echo "#define EXECRCS $e /* variant of execv() to use on subprograms */"

: MERGE
echo "#define MERGE \"${RCSPREFIX}merge\" /* name of 'merge' program */"

: RCSDIR, SLASH, TMPDIR
rm -fr a.RCS && mkdir a.RCS && echo x >a.RCS/x &&
cat >a.c <<'EOF' || exit
#include "a.h"
main() { exitmain(!fopen(NAME,"r")); }
EOF
for NAME in a.RCS/x 'a.rcs/x' 'A.RCS\\x' bad
do ($C -DNAME="\"$NAME\"" a.c $L && ./a.out) && break
done
case $NAME in
a.RCS/x) RCSDIR=RCS/ SLASH=/ TMPDIR=/tmp/;;
a.rcs/x) RCSDIR=rcs/ SLASH=/ TMPDIR=/tmp/;;
'A.RCS\\X') RCSDIR='RCS\\' SLASH='\\' TMPDIR='\\TMP\\';;
*) echo >&2 "cannot deduce RCSDIR"; exit 1;;
esac
rm -fr a.RCS
echo "#define RCSDIR \"$RCSDIR\" /* subdirectory for RCS files */"
echo "#define SLASH '$SLASH' /* path name separator */"
echo "#define TMPDIR \"$TMPDIR\" /* default directory for temporary files */"

: DIFF_PATH_HARDWIRED
case $DIFF in
"$SLASH"*) h=1;;
*) h=0
esac
echo "#define DIFF_PATH_HARDWIRED $h /* Is DIFF absolute, not relative?  */"

: ROOTPATH
case `pwd` in
[/\\]*)
	echo '#define ROOTPATH(p) ((p)[0]==SLASH)';;
?:[/\\]*)
	echo '#define ROOTPATH(p) ((p)[0] && (p)[1]==':' && (p)[2]==SLASH)';;
*)
	echo >&2 "cannot deduce ROOTPATH"; exit 1;;
esac

: RCSSEP
if echo x >a.1234567890,v && ($C -DNAME=\"a.1234567890,v\" a.c $L && ./a.out)
then RCSSEP=','
else RCSSEP='\0'
fi
echo "#define RCSSEP '$RCSSEP' /* separator for RCSSUF */"

: SENDMAIL
echo "#define SENDMAIL $SENDMAIL /* how to send mail */"

: fprintf, printf, vfprintf
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef printf
	int printf P((const char*,...));
#endif
int main() { printf(""); exitmain(0); }
EOF
if $C -S a.c >&2
then a='1 /* These agree with <stdio.h>.  */'
else a='0 /* These conflict with <stdio.h>.  */'
fi
cat <<EOF || exit
#if $a
	int fprintf P((FILE*,const char*,...));
	int printf P((const char*,...));
#	if has_vfprintf
		int vfprintf P((FILE*,const char*,...));
#	else
		void _doprnt P((const char*,...));
#	endif
#endif
EOF

: sprintf and other routines with '...' and default promotion problems
cat >a.c <<'EOF' || exit
#include "a.h"
#ifndef sprintf
	int sprintf();
#endif
int main()
{
	char buf[1];
	exitmain(sprintf(buf, "") != 0);
}
EOF
if ($C a.c $L && ./a.out) >&2
then t='int '
else t='char *'
fi
cat >a.c <<'EOF' || exit
#include "a.h"
#if has_sys_wait_h
#	include <sys/wait.h>
#endif
declaration
int main() { exitmain(0); }
EOF
for declaration in \
	"${t}sprintf P((char*,const char*,...));" \
	'int chmod P((const char*,mode_t));' \
	'int fcntl P((int,int,...));' \
	'int open P((const char*,int,...));' \
	'mode_t umask P((mode_t));' \
	'pid_t wait P((int*));'
do
	if $C -S -Ddeclaration="$declaration" a.c >&2
	then echo "$declaration"
	else echo "/* $declaration */"
	fi
done
for i in '
#ifndef O_CREAT
	int creat P((const char*,mode_t));
#endif' '
#if has_seteuid
	int setegid P((gid_t));
	int seteuid P((uid_t));
#endif'
	# See declare_getpwuid for how getpwuid() is handled.
do
	echo '#include "a.h"
		int main() { exitmain(0); }'"$i" >a.c || exit
	if $C -S a.c >&2
	then sed 1,2d a.c
	else sed '1,2d; s|.*|/* & */|' a.c
	fi
done
