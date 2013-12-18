/**
*
* Title:  Helios Shell - Header file
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/unix.h,v 1.2 1993/08/12 15:56:06 nickc Exp $
*
**/
#include <signal.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define strrchr rindex

#define FALSE 0
#define TRUE  1

typedef int BOOL;
typedef struct direct DIRENT;

typedef struct node
{
  struct node *Next;
  struct node *Prev;
} NODE;

typedef struct list
{
  NODE *Head;
  NODE *Earth;
  NODE *Tail;
} LIST;

#define forever   for (;;)
#define unless(c) if(!(c))
#define until(c)  while(!(c))

#ifdef PATH_MAX
#undef PATH_MAX
#endif

#define PATH_MAX 1024

#define ENVCHAR ':'

#define SHELL_CMD    "/usr/perihelion/bin/shell"
#define LOGIN_CMD    "/bin/Login"
#define LOGIN_FILE   ".login"
#define CSHRC_FILE   ".cshrc"
#define HISTORY_FILE ".history"
#define LOGOUT_FILE  ".logout"
#define TEMP_FILE    "/tmp/shell"

#define DEBUG if (debugging) putmessage
#define sysinit()
#define systidy()
#define termgetc(f) fgetc(f)
#define unixpath(p)
#define syspath(p)
#define isabspath(p) ((p)[0] == '/')
#define catch()  setjmp(home)
#define throw(c) longjmp(home, c)
#define wait2(s, o) wait3(s, o, 0)
#define raise(s) kill(0, s)
