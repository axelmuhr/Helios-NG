/*
 * Spawn. New version, which
 * interracts with the job control stuff
 * in the 4.X BSD C shell.
 * Last edit:  Wed Aug 27 11:16:07 PDT 1986
 * By:	       rtech!daveb, to use stop for ksh.
 */
#include	"def.h"

#include	<sgtty.h>
#include	<signal.h>
#include	<sys/wait.h>

char	*shellp = NULL;			/* Saved "SHELL" name.		*/

extern	struct	sgttyb	oldtty;		/* There really should be a	*/
extern	struct	sgttyb	newtty;		/* nicer way of doing this, so	*/
extern	struct	sgttyb	oldtchars;	/* spawn does not need to know	*/
extern	struct	sgttyb	newtchars;	/* about the insides of the	*/
extern	struct	sgttyb	oldltchars;	/* terminal I/O code.		*/
extern	struct	sgttyb	newltchars;

extern	char	*getenv();

/*
 * This code does a one of 2 different
 * things, depending on what version of the shell
 * you are using. If you are using the C shell, which
 * implies that you are using job control, then MicroEMACS
 * moves the cursor to a nice place and sends itself a
 * stop signal. If you are using the Bourne shell it runs
 * a subshell using fork/exec. Bound to "C-C", and used
 * as a subcommand by "C-Z".
 *
 * Daveb -- changed sense of test so that we only spawn if you
 *	    are explicitly using /bin/sh.  This makes it stop
 *	    work with the ksh.
 */
/*ARGSUSED*/
spawncli(f, n) {
	register int	pid, wpid, (*oqsig)(), (*oisig)(), omask;
	union wait	status;

	if (shellp == NULL) {
		shellp = getenv("SHELL");
		if (shellp == NULL)
			shellp = getenv("shell");
		if (shellp == NULL)
			shellp = "/bin/sh";	/* Safer.		*/
	}
	ttcolor(CTEXT);
	ttnowindow();
	if (strcmp(shellp, "/bin/csh") == 0) {
		if (epresf != FALSE) {
			ttmove(nrow-1, 0);
			tteeol();
			epresf = FALSE;
		}				/* Csh types a "\n"	*/
		ttmove(nrow-2, 0);		/* before "Stopped".	*/
	} else {
		ttmove(nrow-1, 0);
		if (epresf != FALSE) {
			tteeol();
			epresf = FALSE;
		}
	}
	if (ttcooked() == FALSE)
		return (FALSE);
	if (strcmp(shellp, "/bin/sh") != 0) {	/* C shell, ksh		*/
		omask = sigsetmask(0);
		(void) kill(0, SIGTSTP);
		(void) sigsetmask(omask);
	} else {				/* Bourne shell.	*/
		oqsig = signal(SIGQUIT, SIG_IGN);
		oisig = signal(SIGINT,	SIG_IGN);
		if ((pid=fork()) < 0) {
			(void) signal(SIGQUIT, oqsig);
			(void) signal(SIGINT,  oisig);
			ewprintf("Failed to create process");
			return (FALSE);
		}
		if (pid == 0) {
			execl(shellp, "sh", "-i", NULL);
			_exit(0);		/* Should do better!	*/
		}
		while ((wpid=wait(&status))>=0 && wpid!=pid)
			;
		(void) signal(SIGQUIT, oqsig);
		(void) signal(SIGINT,  oisig);
	}
	sgarbf = TRUE;				/* Force repaint.	*/
	return ttraw();
}
