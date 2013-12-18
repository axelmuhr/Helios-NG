/**
*
* Title:  Helios Shell - Signal handling.
*
* Author: Andy England
*
* Date:   June 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/signal.c,v 1.9 1993/08/12 15:55:23 nickc Exp $
*
**/
#include "shell.h"


BOOL testbreak()
{
  if (breakflag)
  {
    breakflag = FALSE;
    unless(login || interactive)
	exitflag = TRUE;
    return TRUE;
  }
  return FALSE;
}

void siginit()
{
#ifdef UNIX 
  struct sigvec vec;

  vec.sv_handler = sighandler;
  vec.sv_mask = 0;
  vec.sv_onstack = 0;
  ignore sigvec(SIGINT, &vec, NULL);
#ifdef JOBOP
  ignore sigvec(SIGTSTP, &vec, NULL);
  ignore sigvec(SIGCHLD, &vec, NULL);
  ignore sigvec(SIGTTIN, &vec, NULL);
  ignore sigvec(SIGTTOU, &vec, NULL);
#endif
#else
  struct sigaction act;

  ctrlcbegin();
  act.sa_handler = sighandler;
  act.sa_mask = 0;
  act.sa_flags = 0;
  ignore sigaction(SIGINT, &act, NULL);

#ifdef JOBOP
  ignore sigaction(SIGTSTP, &act, NULL);
  ignore sigaction(SIGCHLD, &act, NULL);
  ignore sigaction(SIGTTIN, &act, NULL);
  ignore sigaction(SIGTTOU, &act, NULL);
#endif
#endif
}

void sighandler(int sig)
{
#ifdef DEBUGGING
	DEBUG("sighandler(%d)",sig);
#endif
  switch (sig)
  {
    case SIGINT:
    breakflag = TRUE;
#ifdef HELIOS
    putctrlc();
#else
    unless (waitwrpid == 0) kill(waitwrpid, SIGINT);
#endif
    return;

#ifdef JOBOP
    case SIGTSTP:
    unless (waitwrpid == 0) kill(waitwrpid, SIGSTOP);
    waitwrpid = 0;
    return;

    case SIGALRM:
    ignore error(ERR_AUTOLOGOUT, NULL);
    logout(OK);
    return;

    case SIGCHLD:
    {
      int pid, status;

      until (((pid = wait2(&status, WNOHANG)) == 0) OR (pid == -1))
        notifyproc(pid, status);
    }
    return;

    case SIGTTIN:
    fprintf(stderr, "Background read\n");
    return;

    case SIGTTOU:
    fprintf(stderr, "Background write\n");
    return;
#endif

    default:
    return;
  }
}

