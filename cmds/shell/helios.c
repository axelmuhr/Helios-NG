/**
*
* Title:  Helios Shell - Helios dependent parts.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/helios.c,v 1.9 1993/08/12 15:55:16 nickc Exp $
*
**/
#include "shell.h"
#include <helios.h>
#include <signal.h>
#include <attrib.h>
#include <ioevents.h>
#include <message.h>
#include <root.h>
#include <fault.h>
#include <termios.h>

#define SYSERR_MAX 36
int sys_nerr = SYSERR_MAX;
char *sys_errlist[SYSERR_MAX + 1] =
{
  "Error 0",
  "Arg list too long",
  "Permission denied",
  "Resource temporarily unavailable",
  "Bad file number",
  "Resource busy",
  "No child process",
  "Resource deadlock would occur",
  "Domain error",
  "File exists",
  "Bad address",
  "File too large",
  "Interrupted system call",
  "Invalid argument",
  "I/O error",
  "Is a directory",
  "Too many files",
  "Too many links",
  "Filename too long",
  "Too many open files",
  "No such device",
  "No such file or directory",
  "Exec format error",
  "No locks available",
  "Not enough space",
  "No space left on device",
  "Not a directory",
  "Directory not empty",
  "Inappropriate I/O control operation",
  "No such device or address",
  "Operation not permitted",
  "Broken pipe",
  "Result too large",
  "Read-only file system",
  "Invalid seek",
  "No such process",
  "Improper link"
};

PUBLIC int fifo(int fds[2])
{
  char fifoname[NUMSTR_MAX + 12];
  static int fifocount = 0;

  sprintf(fifoname, "/fifo/pipe.%d", fifocount++);
  if ((fds[WRITE] = open(fifoname, O_WRONLY | O_CREAT)) == -1) return -1;
  if ((fds[READ] = open(fifoname, O_RDONLY)) == -1)
  {
    close(fds[WRITE]);
    return -1;
  }
  return 0;
}

PUBLIC BOOL isdir(DIRENT *dirent, char *path)
{
  return ((dirent->d_type & Type_Directory) == Type_Directory) ? TRUE : FALSE;
  path = path;				/* keep the compiler happy...	*/
}

PUBLIC BOOL isexec(DIRENT *dirent, char *path)
{
  /* JMP used to check for LTaskForce as well */
  return ((dirent->d_type & Type_Directory) == Type_Directory) ? FALSE : TRUE;
  path = path;				/* keep the compiler happy...	*/
}

PRIVATE Stream *console;

PUBLIC void sysinit(void)
{
  terminit();
  cooked();
  initenv();
}

PUBLIC void ctrlcbegin(void)
{
#ifdef DEBUGGING
	DEBUG("ctrlcbegin()");
#endif
	tcsetpgrp(0,getpid());
}

PUBLIC void putctrlc()
{
    Attributes attr;
    GetAttributes(console, &attr);
    RemoveAttribute(&attr, ConsolePause);
    SetAttributes(console, &attr);
    Write(console, "^C\n", 3, -1);
}


PUBLIC void systidy(void)
{
  freeenv();
}

PUBLIC void initenv(void)
{
  int count = 0;
  char **envp = environ;
  char *env;

  environ = (char **)newmemory(sizeof(char *) * (lenargv(envp) + 1));
  until ((env = *envp++) == NULL) environ[count++] = strdup(env);
  environ[count] = NULL;
}

PUBLIC void freeenv(void)
{
  char **envp = environ;
  char *env;

  until ((env = *envp++) == NULL) freememory((int *)env);
  freememory((int *)environ);
}

PUBLIC char *newenv(char *name, char *value)
{
  int length = strlen(name);
  char *env = (char *)newmemory(length + strlen(value) + 2);

  strcpy(env, name);
  env[length] = '=';
  strcpy(env + length + 1, value);
  return env;
}

PUBLIC void setenv(char *name, char *value)
{
  int count = 0;
  int length = strlen(name);
  char *env;
  char **newenviron;

  until ((env = environ[count]) == NULL)
  {
    if (strnequ(name, env, length) AND env[length] == '=')
    {
      freememory((int *)env);
      environ[count] = newenv(name, value);
      return;
    }
    count++;
  }
  newenviron = (char **)newmemory(sizeof(char *) * (count + 2));
  memmove(newenviron, environ, sizeof(char *) * count);
  newenviron[count] = newenv(name, value);
  newenviron[count + 1] = NULL;
  freememory((int *)environ);
  environ = newenviron;
}

PUBLIC void delenv(char *name)
{
  int count = 0;
  int length = strlen(name);
  int offset = -1;
  char *env;
  char **newenviron;

  until ((env = environ[count]) == NULL)
  {
    if (strnequ(name, env, length) AND env[length] == '=') offset = count;
    count++;
  }
  unless (offset == -1)
  {
    newenviron = (char **)newmemory(sizeof(char *) * count);
    memmove(newenviron, environ, sizeof(char *) * offset);
    memmove(newenviron + offset, environ +  offset + 1,
      sizeof(char *) * (count - offset));
    freememory((int *)environ[offset]);
    freememory((int *)environ);
    environ = newenviron;
  }
}

PRIVATE Attributes cookedattr, rawattr;

PUBLIC void terminit(void)
{
  setvbuf(stdin, NULL, _IONBF, 0);
  console = Heliosno(stdin);
  GetAttributes(console, &cookedattr);
  rawattr = cookedattr;
  RemoveAttribute(&rawattr, ConsoleEcho);
  RemoveAttribute(&rawattr, ConsolePause);
  RemoveAttribute(&rawattr, ConsoleIgnoreBreak);
  RemoveAttribute(&rawattr, ConsoleBreakInterrupt);
  RemoveAttribute(&rawattr, ConsoleRawOutput);
  AddAttribute(&rawattr, ConsoleRawInput);
}

PUBLIC void raw(void)
{
  fflush(stdout);
  SetAttributes(console, &rawattr);
}

PUBLIC int termgetc(FILE *file)
{
  int c;

  if ((c = fgetc(file)) == EOF)
  {
    clearerr(file);
    c = CTRL_D;
  }
  return c;
}

PUBLIC void cooked(void)
{
  fflush(stdout);
  SetAttributes(console, &cookedattr);
}

PUBLIC void fault(unsigned long code)
{
  char msg[128];

  Fault(code, msg, 128);
  printf("%08lx: %s.\n", code, msg);
}
