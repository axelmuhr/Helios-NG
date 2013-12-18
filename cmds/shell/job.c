/**
*
* Title:  Helios Shell - Job Control.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /users/nickc/RTNucleus/cmds/shell/RCS/job.c,v 1.5 1994/03/21 16:39:08 nickc Exp $
*
**/
#include "shell.h"
#include <sys/wait.h>

JOB jobtable[JOBS_MAX];
LIST joblist;

#define RUNNING 0xffff0000
#if  !defined(UNIX) && !defined(__HELIOS)
#define NSIG 31
#endif

void newjob(
	    CMD *cmd,
	    int pid )
{
  JOB *job;

#ifdef DEBUGGING
  DEBUG("newjob()");
#endif
  for (job = &jobtable[1]; (job - jobtable) < JOBS_MAX; job++)
  {
    if (job->cmd == NULL)
    {
#ifdef SYSDEB
      if (job->pid == 0)
	job->next = job->prev = job;
#endif 
      AddHead(&joblist, (NODE *)job);
      job->cmd = dupcmd(cmd);
      job->status = RUNNING;
      job->notify = FALSE;
      job->pending = FALSE;
      job->pid = pid;
      fprintf(stderr, "[%d] %d\n", job - jobtable, pid);
      return;
    }
  }
  fprintf(stderr, "No more Jobs\n");
}

void freejob(JOB *job)
{
 Remove((NODE *)job);
  freecmd(job->cmd);
  job->cmd = NULL;
}

void killjob(JOB *job)
{
  if (job->status == RUNNING) kill(job->pid, SIGTERM);
}

JOB *currentjob()
{
  JOB *job = (JOB *)joblist.Head;

  return (job->next == NULL) ? NULL : job;
}

JOB *previousjob()
{
  JOB *job = ((JOB *)joblist.Head)->next;

  return (job == NULL) ? NULL : (job->next == NULL) ? NULL : job;
}

JOB *findjob(int pid)
{
  JOB *job;

  for (job = &jobtable[1]; (job - jobtable) < JOBS_MAX; job++)
  {
    unless (job->cmd == NULL)
    {
      if (pid == job->pid) return job;
    }
  }
  return NULL;
}

JOB *getjob(int jobnumber)
{
  if (jobnumber > JOBS_MAX) return NULL;
  if (jobtable[jobnumber].cmd == NULL) return NULL;
  return &jobtable[jobnumber];
}

static char *sigmsg[NSIG + 1] =
{
#ifdef UNIX
  "",
  "Hangup",
  "Interrupt",
  "Quit",
  "Illegal instruction",
  "Trace trap",
  "IOT instruction",
  "EMT instruction",
  "Floating Point exception",
  "Kill",
  "Bus Error",
  "Segmentation fault",
  "Bad arg to system call",
  "Write on pipe",
  "Alarm clock",
  "Termination signal",
  "User signal 1",
  "User signal 2",
  "Death of a child",
  "Power fail"
#else
  "",
  "Abort",
  "Floating Point exception",
  "Illegal instruction",
  "Interrupt",
  "Segmentation fault",
  "Terminate signal",
  "Stack overflow",
  "Alarm clock",
  "Hangup",
  "Broken pipe",
  "Quit",
  "Trace trap",
  "User signal 1",
  "User signal 2",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "Kill"
#endif
};

void putstatus(int status)
{
  unless (lowbyte(status) == 0 OR (status & 0177) == SIGINT)
  {
    int code;

    if ((code = status & 0177) <= NSIG AND sigmsg[code] != NULL)
      printf("%s", sigmsg[code]);
    else printf("Signal #%d", code);
    if ((status & 0200) == 0200)
#ifdef UNIX
      printf(" (core dumped)");
#else
      printf(" (abnormal termination)");
#endif
    printf("\n");
  }
}

void putjob(JOB *job)
{
  int status = job->status;
  int code;

  printf("[%d]", job - jobtable);
  if (status == RUNNING)
  {
    if (job == currentjob()) printf("  +");
    else if (job == previousjob()) printf("  -");
    printf("\tRunning\t\t");
  }
  else
  {
    printf("\t");
    if (lowbyte(status) == 0)
    {
      if ((code = highbyte(status)) == 0) printf("Done\t\t");
      else printf("Exit %d\t\t", code);
    }
    else
    {
      if ((code = status & 0177) <= NSIG AND sigmsg[code] != NULL)
        printf("%s", sigmsg[code]);
      else printf("Signal #%d", code);
    }
  }
  printf("\t");
  putcmd(job->cmd);
  if ((status & 0200) == 0200)
    printf(" (abnormal termination)");
  printf("\n");
  unless (job->status == RUNNING) freejob(job);
}

void notifyjob(
	       int pid,
	       int status )
{
  JOB *job;

#ifdef DEBUGGING
  DEBUG("notifyjob(%d, %d)", pid, status);
#endif
  if ((job = findjob(pid)) == NULL)
  {
    putstatus(status);
    return;
  }
  job->status = status;
  if (findvar("notify") OR job->notify) putjob(job);
  else job->pending = TRUE;
}

void pendingjobs()
{
  JOB *job;
#ifdef POSIX
  int pid, status;

#ifdef DEBUGGING
  DEBUG("pendingjobs()");
#endif
  until ((pid = wait2(&status, WNOHANG)) == 0 OR pid == -1)
    notifyjob(pid, status);
#endif
  for (job = &jobtable[1]; (job - jobtable) < JOBS_MAX; job++)
  {
    unless (job->cmd == NULL)
    {
      if (job->pending) putjob(job);
    }
  }
}

void putjobtable()
{
  JOB *job;

  for (job = &jobtable[1]; (job - jobtable) < JOBS_MAX; job++)
  {
    unless (job->cmd == NULL) putjob(job);
  }
}
