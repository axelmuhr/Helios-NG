/*
 *	Do the actual making for make
 */

/* RCSID :$Header: /users/nickc/RTNucleus/cmds/make/RCS/make.c,v 1.7 1994/03/08 14:37:29 nickc Exp $ */

#ifdef __HELIOS
#define __system_io
typedef int sysbase;
typedef int FILEHANDLE;
#endif

#include <stdio.h>
#ifdef unix
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#endif
#ifdef eon
#include <sys/stat.h>
#include <sys/err.h>
#endif
#ifdef os9
#include <time.h>
#include <os9.h>
#include <modes.h>
#include <direct.h>
#include <errno.h>
#endif
#ifdef amiga
#include <ctype.h>
#include <errno.h>
#include <libraries/dosextens.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/ports.h>
#include <functions.h>
#endif
#ifdef __HELIOS
#include <ctype.h>
#include <errno.h>
#include <syslib.h>
#include <nonansi.h>
#include <signal.h>
#include <posix.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#endif
#include "h.h"


static int child = -1;
static int signal_received = 0;

#ifdef __HELIOS
extern void expand( char * );
extern bool dyndep( struct name * np );
void make1( struct name * np, struct line * lp, struct depend * qdp );
#endif

void signal_handler(int x)
{ 
  if (child != -1)
   { 
     signal_received = 1;
     kill(child, x);
   }
}

/*
 *	Exec a shell that returns exit status correctly (/bin/esh).
 *	The standard EON shell returns the process number of the last
 *	async command, used by the debugger (ugg).
 *	[exec on eon is like a fork+exec on unix]
 */
int
dosh(text, shell)
    char           *text;
    char           *shell;
{ int result, sigs_installed, temp;
  struct sigaction old, New;

  New.sa_handler = (void(*)())&signal_handler;
  New.sa_mask    = 0;
  New.sa_flags   = 0;

  sigs_installed = sigaction(SIGINT, &New, &old);
  if (sigs_installed == -1)
   perror("warning, failed to set up signal handler");
   
  child = vfork();
  if (child == 0)   /* in the child */
   { int newstream = open("/", O_RDONLY);  /* zap stdin for the shell */
     dup2(newstream, 0);

     if (execlp("shell", "shell", "-fec", text, NULL))
      { perror("failed to execute shell"); exit(1); }
   }

  if (child < 0)
   { perror("failed to vfork child process"); exit(1); }

  temp = wait(&result);

  if (sigs_installed != -1)
   sigaction(SIGINT, &old, Null(struct sigaction)); 
     
  return(result);
  
  shell = shell;
}

/*
 *	Do commands to make a target
 */
void
docmds1(np, lp)
    struct name    *np;
    struct line    *lp;
{
    bool            ssilent;
    bool            signore;
    int             estat;
    register char  *q;
    register char  *p;
    char           *shell;
    register struct cmd *cp;
#ifdef amiga
    long SetSignal();
#endif


    if (*(shell = getmacro("SHELL")) == '\0')
#ifdef eon
	shell = ":bin/esh";
#endif
#ifdef unix
    shell = "/bin/sh";
#endif
#ifdef os9
    shell = "shell";
#endif
#ifdef amiga
    shell = NULL;
#endif
#ifdef __HELIOS
    shell = "/helios/bin/shell";
#endif

    for (cp = lp->l_cmd; cp; cp = cp->c_next) {
	strcpy(str1, cp->c_cmd);
	expand(str1);
	q = str1;
	ssilent = silent;
	signore = ignore;
	while ((*q == '@') || (*q == '-')) {
	    if (*q == '@')	/* Specific silent  */
		ssilent = TRUE;
	    else		/* Specific ignore  */
		signore = TRUE;
	    q++;		/* Not part of the command  */
	}

	if (!domake)
	    ssilent = 0;

	if (!ssilent)
	    fputs("    ", stdout);

	for (p = q; *p; p++) {
	    if (*p == '\n' && p[1] != '\0') {
		*p = ' ';
		if (!ssilent)
		    fputs("\\\n", stdout);
	    } else if (!ssilent)
		putchar(*p);
	}
	if (!ssilent)
	    putchar('\n');

	if (domake) {		/* Get the shell to execute it  */
	    if ((estat = dosh(q, shell)) != 0) {
#ifdef __HELIOS
		    printf("%s: Error code %x", myname, estat);
#else
		    printf("%s: Error code %d", myname, estat);
#endif
		    if (signore)
			fputs(" (Ignored)\n", stdout);
		    else {
			putchar('\n');
			if (!(np->n_flag & N_PREC))
			    if (unlink(np->n_name) == 0)
				printf("%s: '%s' removed.\n", myname, np->n_name);
                        if ((estat & 0x0FF) == 0)
                         exit(estat >> 8);
                        else
			 exit(estat);
		    }
	    }
#ifdef amiga
	    if ((SetSignal(0L, SIGBREAKF_CTRL_D) & SIGBREAKF_CTRL_D) != 0) {
		fputs("make: abort due to ^D\n", stdout);
		exit(1);
	    }
#endif	    
	}
    }
}

void
docmds(np)
    struct name    *np;
{
    register struct line *lp;


    for (lp = np->n_line; lp; lp = lp->l_next)
	docmds1(np, lp);
}


#ifdef os9
/*
 *	Some stuffing around to get the modified time of a file
 *	in an os9 file system
 */
getmdate(fd, tbp)
    struct sgtbuf  *tbp;
{
    struct registers regs;
    static struct fildes fdbuf;


    regs.rg_a = fd;
    regs.rg_b = SS_FD;
    regs.rg_x = &fdbuf;
    regs.rg_y = sizeof(fdbuf);

    if (_os9(I_GETSTT, &regs) == -1) {
	errno = regs.rg_b & 0xff;
	return -1;
    }
    if (tbp) {
	_strass(tbp, fdbuf.fd_date, sizeof(fdbuf.fd_date));
	tbp->t_second = 0;	/* Files are only acurate to mins */
    }
    return 0;
}


/*
 *	Kludge routine to return an aproximation of how many
 *	seconds since 1980.  Dates will be in order, but will not
 *	be lineer
 */
time_t
cnvtime(tbp)
    struct sgtbuf  *tbp;
{
    long            acc;


    acc = tbp->t_year - 80;	/* Baseyear is 1980 */
    acc = acc * 12 + tbp->t_month;
    acc = acc * 31 + tbp->t_day;
    acc = acc * 24 + tbp->t_hour;
    acc = acc * 60 + tbp->t_minute;
    acc = acc * 60 + tbp->t_second;

    return acc;
}


/*
 *	Get the current time in the internal format
 */
time(tp)
    time_t         *tp;
{
    struct sgtbuf   tbuf;


    if (getime(&tbuf) < 0)
	return -1;

    if (tp)
	*tp = cnvtime(&tbuf);

    return 0;
}
#endif


/*
 *	Get the modification time of a file.  If the first
 *	doesn't exist, it's modtime is set to 0.
 */
void
modtime(np)
    struct name    *np;
{
#ifdef unix
    struct stat     info;
    int             fd;
    if (stat(np->n_name, &info) < 0) {
	if (errno != ENOENT)
	    fatal("Can't open %s; error %d", np->n_name, errno);

	np->n_time = 0L;
    } else
	np->n_time = info.st_mtime;
#endif
#ifdef eon
    struct stat     info;
    int             fd;


    if ((fd = open(np->n_name, 0)) < 0) {
	if (errno != ER_NOTF)
	    fatal("Can't open %s; error %02x", np->n_name, errno);

	np->n_time = 0L;
    } else if (getstat(fd, &info) < 0)
	fatal("Can't getstat %s; error %02x", np->n_name, errno);
    else
	np->n_time = info.st_mod;

    close(fd);
#endif
#ifdef os9
    struct sgtbuf   info;
    int             fd;


    if ((fd = open(np->n_name, 0)) < 0) {
	if (errno != E_PNNF)
	    fatal("Can't open %s; error %02x", np->n_name, errno);

	np->n_time = 0L;
    } else if (getmdate(fd, &info) < 0)
	fatal("Can't getstat %s; error %02x", np->n_name, errno);
    else
	np->n_time = cnvtime(&info);

    close(fd);
#endif
#ifdef amiga
    struct FileInfoBlock *fib;
    struct Lock *myLock;
    long ioErr;

    fib = (struct FileInfoBlock *) malloc((unsigned) sizeof(struct FileInfoBlock));
    if ((myLock = Lock(np->n_name, ACCESS_READ)) == NULL) {
	if ((ioErr = IoErr()) != ERROR_OBJECT_NOT_FOUND)
	    fatal("Can't Lock '%s'; error %3ld", np->n_name, ioErr);
	np->n_time = 0L;
    } else if (!Examine(myLock, fib)) {
	UnLock(myLock);
	fatal("Can't Examine '%s'; error %3ld", np->n_name, IoErr());
    } else {
 	np->n_time = fib->fib_Date.ds_Tick/TICKS_PER_SECOND +
		 60*fib->fib_Date.ds_Minute + 86400*fib->fib_Date.ds_Days;
    	UnLock(myLock);
    }
    free((char *) fib);
#endif
#ifdef __HELIOS
    ObjInfo info;

/*
   This call to Locate should not be necessary but ObjectInfo does not
   like being used on a non-existent object
*/
    if (Locate(CurrentDir, np->n_name) == Null(Object))
	np->n_time = 0L;
    else if (ObjectInfo(CurrentDir, np->n_name, (byte *)&info))
        np->n_time = 0L;
    else
	np->n_time = info.Dates.Modified;
#endif
}


/*
 *	Update the mod time of a file to now.
 */
void
touch(np)
    struct name    *np;
{
#ifndef __HELIOS
    char            c;
    int             fd;
#endif

    if (!domake || !silent)
	printf("    touch(%s)\n", np->n_name);

    if (domake) {
#ifdef unix
	long            a[2];

	a[0] = a[1] = time(0);
	if (utime(np->n_name, &a[0]) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		   myname, np->n_name);
#endif
#ifdef eon
	if ((fd = open(np->n_name, 0)) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		   myname, np->n_name);
	else {
	    uread(fd, &c, 1, 0);
	    uwrite(fd, &c, 1);
	}
	close(fd);
#endif
#ifdef os9
	/*
	 * Strange that something almost as totally useless as this is easy
	 * to do in os9! 
	 */
	if ((fd = open(np->n_name, S_IWRITE)) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		   myname, np->n_name);
	close(fd);
#endif
#ifdef amiga
	struct MsgPort *task;
	ULONG dateStamp[3];
	struct Lock *lock, *plock;
	UBYTE *pointer;

	if(!(pointer= (UBYTE *)AllocMem(64L,MEMF_PUBLIC)))
		fatal("Can't get 64 bytes for pointer");
	if(!(task=(struct MsgPort *)DeviceProc(np->n_name))) {
		printf("%s: can't get MsgPort for '%s'\n", myname, np->n_name);
		FreeMem((void *) pointer, 64L);
		return;
	}
	if(!(lock = Lock(np->n_name,SHARED_LOCK))) {
		printf("%s: '%s' not touched - non-existant\n",
			myname, np->n_name);
		FreeMem((void *) pointer, 64L);
		return;
	}
	plock = ParentDir(lock);
	UnLock(lock);

	strcpy((pointer + 1),np->n_name);
	*pointer = strlen(np->n_name);
 
	dos_packet(task, ACTION_SET_DATE, NULL, plock, (ULONG) &pointer[0] >> 2,
		(ULONG) DateStamp(dateStamp), 0L, 0L, 0L);

	UnLock(plock);
	FreeMem((void *) pointer, 64L);
#endif
#ifdef __HELIOS
	Date 	  date = GetDate();
	DateSet	  dset;	 	/* XXX - added by NC 24/6/92 */

	dset.Access   = date;	/* XXX - added by NC 24/6/92 */
	dset.Creation = 0;	/* XXX - added by NC 24/6/92 */
	dset.Modified = 0;	/* XXX - added by NC 24/6/92 */
	
	if (SetDate(CurrentDir, np->n_name, &dset))
	    printf("%s: '%s' not touched - non-existent\n",
		   myname, np->n_name);
#endif
    }
}

/*
 * Recursive routine to make a target. 
 */
int
make(np, level)
    struct name    *np;
    int             level;
{
    register struct depend *dp;
    register struct line *lp;
    register struct depend *qdp;
    time_t          dtime = 1;
    bool            didsomething = 0;

    if (np->n_flag & N_DONE)
	return 0;

    if (!np->n_time)
	modtime(np);		/* Gets modtime of this file  */

    if (rules) {
	for (lp = np->n_line; lp; lp = lp->l_next)
	    if (lp->l_cmd)
		break;

	if (!lp)
	    dyndep(np);
    }
    if (!(np->n_flag & N_TARG) && np->n_time == 0L)
	fatal("Don't know how to make %s", np->n_name);

    for (qdp = (struct depend *) 0, lp = np->n_line; lp; lp = lp->l_next) {
	for (dp = lp->l_dep; dp; dp = dp->d_next) {
	    make(dp->d_name, level + 1);
	    qdp = newdep(dp->d_name, qdp);
	    dtime = max(dtime, dp->d_name->n_time);
	}
	if ((np->n_flag & N_DOUBLE) && (np->n_time < dtime)) {
	 if (!quest)
	  { make1(np, lp, qdp);	/* free()'s qdp */
	    dtime = 1;
	    qdp = (struct depend *) 0;
	  }
	 didsomething++;
	}
    }

    np->n_flag |= N_DONE;

    if (np->n_time < dtime && !(np->n_flag & N_DOUBLE)) {
    	if (!quest)
	 make1(np, (struct line *) 0, qdp);	/* free()'s qdp */
	time((void *)&np->n_time);
	didsomething++;
    } else if (level == 0 && !didsomething && !quest)
	printf("%s: '%s' is up to date\n", myname, np->n_name);

    if (quest)
     return((didsomething > 0) ? 1 : 0);
    else
     return 0;
}

void
make1(np, lp, qdp)
    register struct depend *qdp;
    struct line    *lp;
    struct name    *np;
{
    register struct depend *dp;
    register char *suff;
    static bool	warned_about_str1	= FALSE;
    static bool	warned_about_str2	= FALSE;
    
    if (dotouch)
	touch(np);
    else {
    		/* $< should only be defined here if it is not already	*/
    		/* defined. There is a problem when combining implicit	*/
    		/* rules such as .c.o: with explicit dependencies such	*/
    		/* test.o : test.h. For compatibility $< should refer	*/
    		/* to test.c rather than test.h				*/
    	if (qdp && !(np->n_flag & N_LT_DEFINED))
    	 setmacro("<", qdp->d_name->n_name);

	strcpy(str1, "");
	strcpy(str2, "");
	for (dp = qdp; dp; dp = qdp) {
	    if (dp->d_name->n_time > np->n_time)
	     {		/* BLV - check for buffer overflow	*/
		if ((strlen(str2) + strlen(dp->d_name->n_name) + 2) > LZ)
		{
			unless (warned_about_str2)
				fputs("Warning: out of buffer space for expansion of $?\n", stderr);
			warned_about_str2 = TRUE;
		}
		else
		{			
			if (strlen(str2))
				strcat(str2, " ");
			strcat(str2, dp->d_name->n_name);
		}
	     }
	    if ((strlen(str1) + strlen(dp->d_name->n_name) + 2) > LZ)
	    {
	    	unless (warned_about_str1)
	    		fputs("Warning: out of buffer space for expansion of $^\n", stderr);
	    	warned_about_str1 = TRUE;
	    }
	    else
	    {
		if (strlen(str1))
			strcat(str1, " ");
		strcat(str1, dp->d_name->n_name);
	    }
	    qdp = dp->d_next;
	    free(dp);
	}
	setmacro("^", str1);
	setmacro("@", np->n_name);
	setmacro("?", str2);
	
	strcpy(str1, np->n_name);
	if((suff = suffix(str1))!=NULL) *suff = '\0';
	setmacro("*", str1 );

	if (lp)			/* lp set if doing a :: rule */
	    docmds1(np, lp);
	else
	    docmds(np);
    }
}
#ifdef amiga
/*
 * Replace the Aztec-provided time function with one which returns something
 * easy to find and compare, namely the number of seconds since the Amiga's
 * reference date.  This is the same thing returned by modtime() above.
 */
time_t
time(v)
    time_t *v;
{
    long t[3];

    DateStamp(t);
    t[0] = t[2]/TICKS_PER_SECOND + 60*t[1] + 86400*t[0];
    if (v)
	*v = t[0];
    return t[0];
}
#endif
