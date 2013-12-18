/*	Spawn:	various DOS access commands
		for MicroEMACS

 $Header: /hsrc/cmds/emacs/RCS/spawn.c,v 1.1 1990/08/23 15:16:30 jon Exp $

*/

#include        <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if     AMIGA
#define  NEW   1006
#endif

#if		ST520 & MEGAMAX
#include <osbind.h>
#include <string.h>
#define LOAD_EXEC 0 	/* load and execute the program */
char	*STcmd,		/* the command filename & path  */
	*STargs,	/* command args (if any)        */
	*STenv,		/* environment                  */
	*STwork;	/* work area			*/
#endif

#if     VMS
#define EFN     0                               /* Event flag.          */

#include        <ssdef.h>                       /* Random headers.      */
#include        <stsdef.h>
#include        <descrip.h>
#include        <iodef.h>

extern  int     oldmode[3];                     /* In "termio.c"        */
extern  int     newmode[3];                     /* In "termio.c"        */
extern  short   iochan;                         /* In "termio.c"        */
#endif

#if     V7 | USG | BSD
#include        <signal.h>
extern int vttidy();
#endif

#if	MSDOS & MSC
#include	<process.h>
#define	system(a)	spawnlp(P_WAIT, a, NULL)
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
spawncli(f, n)
int f,n;
{
#if     AMIGA
        long newcli;

#endif

#if     V7 | USG | BSD
        register char *cp;
        char    *getenv();
#endif
#if	HELIOS
	int stat;
#endif
	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	AMIGA
        newcli = Open("CON:0/0/639/199/MicroEmacs Subprocess", NEW);
        mlwrite("[Starting new CLI]");
        sgarbf = TRUE;
        Execute("", newcli, 0);
        Close(newcli);
        return(TRUE);
#endif

#if     VMS
        movecursor(term.t_nrow, 0);             /* In last line.        */
        mlputs("[Starting DCL]\r\n");
        TTflush(); 	                     /* Ignore "ttcol".      */
        sgarbf = TRUE;
        return (sys(NULL));                     /* NULL => DCL.         */
#endif
#if     CPM
        mlwrite("Not in CP/M-86");
#endif
#if	ST520
	mlwrite("Not in TOS");
#endif
#if     MSDOS & AZTEC
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
	TTkclose();
	system("command.com");
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
#endif
#if     MSDOS & LATTICE
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
	TTkclose();
        sys("\\command.com", "");               /* Run CLI.             */
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
#endif
#ifdef HELIOS
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
        TTclose();                              /* stty to old settings */

/* causes exit back to shell after 2-3 secs */
	if( vfork()==0 )
	{
		execl("/helios/bin/shell", "shell", NULL );
		_exit(20);
	}
	wait(&stat);
        sgarbf = TRUE;
        TTopen();
        return(TRUE);
#endif
#if     V7 | USG | BSD
        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
        TTclose();                              /* stty to old settings */
        if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
                system(cp);
        else
#if	BSD
                system("exec /bin/csh");
#else
                system("exec /bin/sh");
#endif
        sgarbf = TRUE;
        sleep(2);
        TTopen();
        return(TRUE);
#endif
	f = f;
	n = n;
}

#if	BSD

bktoshell()		/* suspend MicroEMACS and wait to wake up */
{
	int pid;

	vttidy();
	pid = getpid();
	kill(pid,SIGTSTP);
}

rtfrmshell()
{
	TTopen();
	curwp->w_flag = WFHARD;
	sgarbf = TRUE;
}
#endif

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
spawn(f, n)
int f,n;
{
        register int    s;
        char            line[NLINE];

#if	ST520 & MEGAMAX
	int i,j,k;
	char *sptr,*tptr;
#endif

#if     AMIGA
        long newcli;
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	AMIGA
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return (s);
        newcli = Open("CON:0/0/639/199/MicroEmacs Subprocess", NEW);
        Execute(line,0,newcli);
        Close(newcli);
        tgetc();     /* Pause.               */
        sgarbf = TRUE;
        return(TRUE);
#endif
#if     ST520 & MEGAMAX
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
	movecursor(term.t_nrow - 1, 0);
	TTclose();
/*
 * break the line into the command and its args
 * be cute about it, if there is no '.' in the filename, try
 * to find .prg, .tos or .ttp in that order
 * in any case check to see that the file exists before we run 
 * amok
 */
	STenv = NULL;
	if((tptr = index(&line[0],' ')) == NULL) { /* no args */
		STcmd = (char *)malloc(strlen(line) + 1);
		strcpy(STcmd,line);
		STargs = NULL;
	}
	else {  /* seperate out the args from the command */
		/* resist the temptation to do ptr arithmetic */
		STcmd = (char *)malloc(strlen(line) + 1);
		for(i = 0,sptr = &line[0]; sptr != tptr; sptr++,i++)
			STcmd[i] = *sptr;
		STcmd[i] = '\0';
		for(; *tptr == ' ' || *tptr == '\t'; tptr++);
		if(*tptr == '\0')
			STargs = NULL;
		else {
			STargs = (char *)malloc(strlen(tptr) + 2);
/* first byte of STargs is the length of the string */
			STargs[0] = strlen(tptr);
			STargs[1] = NULL; /* fake it for strcat */
			strcat(STargs,tptr);
		}
	}
/*
 * before we issue the command look for the '.', if it's not there
 * try adding .prg, .tos and .ttp to see if they exist, if not
 * issue the command as is
 */
	if((tptr = index(STcmd,'.')) == NULL) {
 		STwork = (char *)malloc(strlen(STcmd) + 4);
 		strcpy(STwork,STcmd);
 		strcat(STwork,".prg");
 		tptr = index(STwork,'.');
 		if(Fsfirst(1,STwork) != 0) { /* try .tos */
 			strcpy(tptr,".tos");
 			if(Fsfirst(1,STwork) != 0) { /* try .ttp */
 				strcpy(tptr,".ttp");
 				if(Fsfirst(1,STwork) != 0) /* never mind */
 					*STwork = NULL;
 				}
 			}
 	}
 	if(*STwork != NULL)
	        Pexec(LOAD_EXEC,STwork,STargs,STenv); 		
	else
	        Pexec(LOAD_EXEC,STcmd,STargs,STenv);
	TTopen();
        mlputs("\r\n\n[End]");                  /* Pause.               */
        TTgetc();			     /* Pause.               */
        sgarbf = TRUE;
        return (TRUE);
#endif
#if     VMS
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return (s);
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        s = sys(line);                          /* Run the command.     */
        mlputs("\r\n\n[End]");                  /* Pause.               */
        TTflush();
        tgetc();
        sgarbf = TRUE;
        return (s);
#endif
#if     CPM
        mlwrite("Not in CP/M-86");
        return (FALSE);
#endif
#if     MSDOS | (ST520 & LATTICE)
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        mlputs("\r\n\n[End]");
        	tgetc();
        }
        sgarbf = TRUE;
        return (TRUE);
#endif
#if     V7 | USG | BSD | HELIOS
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return (s);
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
        system(line);
        TTopen();
        mlputs("[End - Press Space]");          /* Pause.               */
        TTflush();
        while ((s = tgetc()) != '\r' && s != ' ')
                ;
        sgarbf = TRUE;
        return (TRUE);
#endif
	f = f;
	n = n;
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
pipecmd(f, n)
int f,n;
{
        register int    s;	/* return status from CLI */
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
        char	line[NLINE];	/* command line send to shell */
	static char bname[] = "command";

#if	AMIGA
	static char filnam[] = "ram:command";
        long newcli;
#else
	static char filnam[] = "command";
#endif

#if	MSDOS
	char *tmp;
	char *getenv();
	FILE *fp;
	FILE *fopen();
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	MSDOS
	if ((tmp = getenv("TMP")) == NULL)
		strcpy(filnam, "command");
	else
		strcpy(filnam, tmp);
#endif

#if     VMS
	mlwrite("Not availible under VMS");
	return(FALSE);
#endif
#if     CPM
        mlwrite("Not availible under CP/M-86");
        return(FALSE);
#endif

	/* get the command to pipe in */
        if ((s=mlreply("@", line, NLINE)) != TRUE)
                return(s);

	/* get rid of the command output buffer if it exists */
        if ((bp=bfind(bname, FALSE, 0)) != FALSE) {
		/* try to make sure we are off screen */
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == bp) {
				onlywind(FALSE, 1);
				break;
			}
			wp = wp->w_wndp;
		}
		if (zotbuf(bp) != TRUE)

			return(FALSE);
	}

#if     AMIGA
        newcli = Open("CON:0/0/639/199/MicroEmacs Subprocess", NEW);
	strcat(line, " >");
	strcat(line, filnam);
        Execute(line,0,newcli);
	s = TRUE;
        Close(newcli);
        sgarbf = TRUE;
#endif
#if     MSDOS
	strcat(line," >>");
	strcat(line,filnam);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        system(line);
	TTkopen();
        sgarbf = TRUE;
	if ((fp = fopen(filnam, "r")) == NULL) {
		s = FALSE;
	} else {
		fclose(fp);
		s = TRUE;
	}
#endif
#if     V7 | USG | BSD |HELIOS
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
	strcat(line,">");
	strcat(line,filnam);
        system(line);
        TTopen();
        TTflush();
        sgarbf = TRUE;
        s = TRUE;
#endif

	if (s != TRUE)
		return(s);

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
			return(FALSE);

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE)
		return(FALSE);

	/* make this window in VIEW mode, update all mode lines */
	curwp->w_bufp->b_mode |= MDVIEW;
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}

	/* and get rid of the temporary file */
	unlink(filnam);
	return(TRUE);
	f = f;
	n = n;
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
filter(f, n)
int f,n;
{
        register int    s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
        char line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */
	static char bname1[] = "fltinp";

#if	AMIGA
	static char filnam1[] = "ram:fltinp";
	static char filnam2[] = "ram:fltout";
        long newcli;
#else
	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

#if     VMS
	mlwrite("Not availible under VMS");
	return(FALSE);
#endif
#if     CPM
        mlwrite("Not availible under CP/M-86");
        return(FALSE);
#endif

	/* get the filter name and its args */
        if ((s=mlreply("#", line, NLINE)) != TRUE)
                return(s);

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, bname1);	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1) != TRUE) {
		mlwrite("[Cannot write filter file]");
		strcpy(bp->b_fname, tmpnam);
		return(FALSE);
	}

#if     AMIGA
        newcli = Open("CON:0/0/639/199/MicroEmacs Subprocess", NEW);
	strcat(line, " <ram:fltinp >ram:fltout");
        Execute(line,0,newcli);
	s = TRUE;
        Close(newcli);
        sgarbf = TRUE;
#endif
#if     MSDOS
	strcat(line," <fltinp >fltout");
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        system(line);
	TTkopen();
        sgarbf = TRUE;
	s = TRUE;
#endif
#if     V7 | USG | BSD | HELIOS
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
	strcat(line," <fltinp >fltout");
        system(line);
        TTopen();
        TTflush();
        sgarbf = TRUE;
        s = TRUE;
#endif

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filnam2,FALSE) == FALSE)) {
		mlwrite("[Execution failed]");
		strcpy(bp->b_fname, tmpnam);
		unlink(filnam1);
		unlink(filnam2);
		return(s);
	}

	/* reset file name */
	strcpy(bp->b_fname, tmpnam);	/* restore name */
	bp->b_flag |= BFCHG;		/* flag it as changed */

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return(TRUE);
	f = f;
	n = n;
}

#if     VMS
/*
 * Run a command. The "cmd" is a pointer to a command string, or NULL if you
 * want to run a copy of DCL in the subjob (this is how the standard routine
 * LIB$SPAWN works. You have to do wierd stuff with the terminal on the way in
 * and the way out, because DCL does not want the channel to be in raw mode.
 */
sys(cmd)
register char   *cmd;
{
        struct  dsc$descriptor  cdsc;
        struct  dsc$descriptor  *cdscp;
        long    status;
        long    substatus;
        long    iosb[2];

        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                          oldmode, sizeof(oldmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                return (FALSE);
        cdscp = NULL;                           /* Assume DCL.          */
        if (cmd != NULL) {                      /* Build descriptor.    */
                cdsc.dsc$a_pointer = cmd;
                cdsc.dsc$w_length  = strlen(cmd);
                cdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
                cdsc.dsc$b_class   = DSC$K_CLASS_S;
                cdscp = &cdsc;
        }
        status = LIB$SPAWN(cdscp, 0, 0, 0, 0, 0, &substatus, 0, 0, 0);
        if (status != SS$_NORMAL)
                substatus = status;
        status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
                          newmode, sizeof(newmode), 0, 0, 0, 0);
        if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
                return (FALSE);
        if ((substatus&STS$M_SUCCESS) == 0)     /* Command failed.      */
                return (FALSE);
        return (TRUE);
}
#endif

#if	~AZTEC & MSDOS

/*
 * This routine, once again by Bob McNamara, is a C translation of the "system"
 * routine in the MWC-86 run time library. It differs from the "system" routine
 * in that it does not unconditionally append the string ".exe" to the end of
 * the command name. We needed to do this because we want to be able to spawn
 * off "command.com". We really do not understand what it does, but if you don't
 * do it exactly "malloc" starts doing very very strange things.
 */
sys(cmd, tail)
char    *cmd;
char    *tail;
{
#if MWC_86
        register unsigned n;
        extern   char     *__end;

        n = __end + 15;
        n >>= 4;
        n = ((n + dsreg() + 16) & 0xFFF0) + 16;
        return(execall(cmd, tail, n));
#endif

#if LATTICE
        return(forklp(cmd, tail, (char *)NULL));
#endif

#if	MSC
	return(spawnlp(P_WAIT, cmd, tail, NULL));
#endif
}
#endif

#if	MSDOS & LATTICE
/*	System: a modified version of lattice's system() function
		that detects the proper switchar and uses it
		written by Dana Hogget				*/

system(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
	char *getenv();
	static char *swchar = "/C";	/*  Execution switch  */
	union REGS inregs;	/*  parameters for dos call  */
	union REGS outregs;	/*  Return results from dos call  */
	char *shell;		/*  Name of system command processor  */
	char *p;		/*  Temporary pointer  */
	int ferr;		/*  Error condition if any  */

	/*  get name of system shell  */
	if ((shell = getenv("COMSPEC")) == NULL) {
		return (-1);		/*  No shell located  */
	}

	p = cmd;
	while (isspace(*p)) {		/*  find out if null command */
		p++;
	}

	/**  If the command line is not empty, bring up the shell  **/
	/**  and execute the command.  Otherwise, bring up the     **/
	/**  shell in interactive mode.   **/

	if (p && *p) {
		/**  detect current switch character and us it  **/
		inregs.h.ah = 0x37;	/*  get setting data  */
		inregs.h.al = 0x00;	/*  get switch character  */
		intdos(&inregs, &outregs);
		*swchar = outregs.h.dl;
		ferr = forkl(shell, "command", swchar, cmd, (char *)NULL);
	} else {
		ferr = forkl(shell, "command", (char *)NULL);
	}

	return (ferr ? ferr : wait());
}
#endif
