/**
*
* Title:  CDL Compiler - Task Force Execution.
*
* Author: Andy England
*
* Date:   June 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/
/* static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/exec.c,v 1.7 1993/08/02 12:25:56 nickc Exp $"; */

#include "cdl.h"

#include <helios.h>
#include <syslib.h>
#include <servlib.h>
#include <nonansi.h>
#include <posix.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>


static int pid = -1;

void signal_handler(int x)
{
  if (pid != -1)
   kill(pid, x);
}

int waitforcmd(void)
{
  int wpid, status;
  until ((wpid = wait(&status)) == pid OR ((wpid == -1) && (errno != EINTR)));
  return(status);
}

int exectaskforce()
{
  FILE *file;
  static int tfcount = 1;
  char tfname[NUMSTR_MAX + 14];
  struct sigaction old, new;
  int sigs_installed = -1;
  int exit_status = 0;
  
  new.sa_handler = &signal_handler;
#ifdef RS6000
  SIGINITSET( new.sa_mask );  
#else
  new.sa_mask    = 0;
#endif
  new.sa_flags   = 0;
  sigs_installed = sigaction(SIGINT, &new, &old);
  
  sprintf(tfname, "/fifo/cdl.tf.%d", tfcount++);
  if ((file = fopen(tfname, "wb")) == NULL) fatal("Unable to open fifo");
  putcode(file);
  fclose(file);
  if ((pid = vfork()) == 0)
  {
    char *argv[2];

    argv[0] = tfname;
    argv[1] = NULL;
#ifdef __HELIOS
    _posixflags(PE_BLOCK, PE_RemExecute);
#endif
    execv(tfname, argv);
    perror(tfname);
    _exit(errno);
  }
  if (pid == -1)
  { if (sigs_installed != -1) sigaction(SIGINT, &old, Null(struct sigaction));
    perror(tfname);
    return(EXIT_FAILURE);
  }
  
  exit_status = waitforcmd();
  
  if (sigs_installed != -1)
   sigaction(SIGINT, &old, Null(struct sigaction));

  return(exit_status);
}
