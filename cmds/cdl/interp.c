/**
*
* Title:  CDL Compiler - Interpreter.
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
static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/interp.c,v 1.1 1990/08/28 10:41:18 james Exp $";

#include "cdl.h"

#ifdef helios
#include <helios.h>
#include <syslib.h>
#include <servlib.h>
#include <nonansi.h>
#include <posix.h>

void InterpError(Message, Code)
char *Message;
int Code;
{
  if (Code == 0) fprintf(stderr, "cdl: %s.\n", Message);
  else fprintf(stderr, "cdl: %s - %x.\n", Message, Code);
  Tidyup();
  exit(1);
}

void WaitFor(pid)
int pid;
{
  int wpid, Status;

  until (((wpid = wait(&Status)) == pid) OR (wpid == -1));
}

void CDLInterpret()
{
  static char TFName[] = "/fifo/tf.cdl";
  int pid;

  if ((OutputFile = fopen(TFName, "wb")) == NULL)
    InterpError("Unable to open fifo", 0);
  DEBUG("Opened FIFO\n");
  PutCode();
  DEBUG("Written code\n");
  fclose(OutputFile);
  if ((pid = vfork()) == 0)
  {
    char *Argv[2];

    Argv[0] = TFName;
    Argv[1] = NULL;
    _posixflags(PE_BLOCK, PE_RemExecute);
    execv(TFName, Argv);
    perror(TFName);
    _exit(errno);
  }
  WaitFor(pid);
}
#else
#define open  myopen
#define close myclose
#define dup2  mydup2
#define pipe  mypipe

void OpenFiddle();
void CloseFiddle();
void CreatePipe();
void ClosePipe();
void Invoke();
void MarkCommon();
void CloseCommon();
void Redirect();
char **BuildArgv();

void CDLInterpret()
{
  DEBUG("CDL Interpret");
  OpenFiddle();
  WalkList(&CommonList, CreatePipe);
  CloseFiddle();
  WalkList(&ComponentList, Invoke);
#ifdef DEBUGGING
  fprintf(stderr, "Invoking complete\n");
#endif
  WalkList(&CommonList, ClosePipe);
  until (wait(0) == -1);
}

void OpenFiddle()
{
  while (open("/dev/null", 1) < 7);
}

void CloseFiddle()
{
  int i;

  for (i = 3; i < 8; i++) close(i);
}

void CreatePipe(Common)
COMMON *Common;
{
  if (pipe(Common->fds) == -1) Bug("Unable to create pipe");
}

void ClosePipe(Common)
COMMON *Common;
{
  close(Common->fds[0]);
  close(Common->fds[1]);
}

void Invoke(Component)
COMPONENT *Component;
{
  DEBUG("Invoke");
  if (fork() == 0)
  {
#ifdef DEBUGGING
    fprintf(stderr, "Invoking %s\n", Component->Name);
#endif
    CloseFiddle();
    WalkList(&Component->StreamList, MarkCommon);
    WalkList(&CommonList, CloseCommon);
    WalkList(&Component->StreamList, Redirect);
    execv(Component->Name, BuildArgv(&Component->ArgList));
    _exit(1);
  }
}

void MarkCommon(IStream)
ISTREAM *IStream;
{
  unless (IStream->Common == NULL) IStream->Common->Name = NULL;
}

void CloseCommon(Common)
COMMON *Common;
{
  unless (Common->Name == NULL)
  {
    close(Common->fds[0]);
    close(Common->fds[1]);
  }
}

void Redirect(IStream)
ISTREAM *IStream;
{
  unless (IStream->Common == NULL)
  {
    int Oldfd = IStream->Number;
    int Newfd = IStream->Common->fds[GetStandard(IStream->Mode)];
    int OtherSide = IStream->Common->fds[1 - GetStandard(IStream->Mode)];

    close(OtherSide);
    unless (Oldfd == Newfd)
    {
      dup2(Newfd, Oldfd);
      close(Newfd);
    }
  }
}

char **BuildArgv(List)
LIST *List;
{
  char **Argv;
  ARG *Arg;
  int Index = 0;
  int Length = CountList(List, NULL);

  Argv = (char **)malloc(sizeof(char *) * (Length + 1));
  for (Arg = (ARG *)List->Head; Arg->Next; Arg = Arg->Next)
  {
    Argv[Index++] = strdup(Arg->Name);
  }
  Argv[Index] = NULL;
  return Argv;
}

#undef open
#undef close
#undef close
#undef pipe
#undef dup2

int myopen(name, mode)
char *name;
int mode;
{
  int fd = open(name, mode);

#ifdef DEBUGGING
  fprintf(stderr, "open(%s, %d) -> %d\n", name, mode, fd);
#endif
  return fd;
}

int myclose(fd)
int fd;
{
#ifdef DEBUGGING
  fprintf(stderr, "close(%d)\n", fd);
#endif
  return close(fd);
}

int mypipe(fds)
int fds[];
{
  int error = pipe(fds);

#ifdef DEBUGGING
  fprintf(stderr, "pipe() -> %d, %d\n", fds[0], fds[1]);
#endif
  return error;
}

int mydup2(oldfd, newfd)
int oldfd, newfd;
{
#ifdef DEBUGGING
  fprintf(stderr, "dup2(%d, %d)\n", oldfd, newfd);
#endif
  return dup2(oldfd, newfd);
}
#endif

