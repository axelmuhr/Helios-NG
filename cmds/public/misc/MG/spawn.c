/*
 * Spawn. New version, which
 * interracts with the job control stuff
 * in the 4.X BSD C shell.
 * Last edit:  Wed Aug 27 11:16:07 PDT 1986
 * By:	       rtech!daveb, to use stop for ksh.
 */
#include	"def.h"

#include	<signal.h>
#include	<sys/wait.h>

char	*shellp = NULL;			/* Saved "SHELL" name.		*/

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
spawncli(f, n)
{
  int pid, wpid;
  int (*oqsig)(), (*oisig)();
  int status;

  if (shellp == NULL)
  {
    shellp = getenv("SHELL");
    if (shellp == NULL) shellp = getenv("shell");
    if (shellp == NULL) shellp = "/helios/bin/shell";
  }
  ttcolor(CTEXT);
  ttnowindow();
  oqsig = signal(SIGQUIT, SIG_IGN);
  oisig = signal(SIGINT, SIG_IGN);
  if ((pid = vfork()) < 0)
  {
    (void) signal(SIGQUIT, oqsig);
    (void) signal(SIGINT,  oisig);
    ewprintf("Failed to create process");
    return (FALSE);
  }
  if (pid == 0)
  {
    execl(shellp, "shell", "-i", NULL);
    _exit(0);
  }
  while ((wpid = wait(&status)) >= 0 && wpid != pid);
  (void) signal(SIGQUIT, oqsig);
  (void) signal(SIGINT, oisig);
  sgarbf = TRUE;
  return ttraw();
}

