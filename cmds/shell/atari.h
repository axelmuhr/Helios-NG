/**
*
* Title:  Helios Shell - Header file
*
* Author: Andy England
*
* $Header: /hsrc/cmds/shell/RCS/atari.h,v 1.2 1992/06/29 15:56:42 nickc Exp $
*
**/
#include "_errno.h"
#include "types.h"
#include "dir.h"

#define strrchr rindex

#define FALSE 0
#define TRUE  1

typedef int BOOL;

#define forever   for (;;)
#define unless(c) if(!(c))
#define until(c)  while(!(c))

#define ENVCHAR ','

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   0
#define O_CREAT  0
#define O_APPEND 0
#define O_TRUNC  0
#define O_EXCL   0

#define SHELL_CMD    "/c/bin/shell.prg"
#define LOGIN_CMD    "/c/bin/login.prg"
#define LOGIN_FILE   "login"
#define CSHRC_FILE   "cshrc"
#define HISTORY_FILE "history"
#define LOGOUT_FILE  "logout"
#define TEMP_FILE    "/c/tmp/shell"

#define DEBUG if (debugging) putmessage
#define isinteractive() TRUE
#define sysinit()
#define systidy()
#define terminit()
#define termbegin()
#define termend()
#define catch()  setjmp(home)
#define throw(c) longjmp(home, c)

