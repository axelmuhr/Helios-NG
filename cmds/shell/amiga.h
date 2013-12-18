/**
*
* Title:  Helios Shell - Header file
*
* Author: Andy England
*
* $Header: /hsrc/cmds/shell/RCS/amiga.h,v 1.1 1990/08/23 15:46:56 james Exp $
*
**/

#define void int

#define FALSE 0
#define TRUE  1

typedef int BOOL;

#define forever   for (;;)
#define unless(c) if(!(c))
#define until(c)  while(!(c))

#define ENVCHAR ':'

#define SHELL_CMD    "/c/shell"
#define LOGIN_CMD    "/c/login"
#define LOGIN_FILE   "login"
#define CSHRC_FILE   "cshrc"
#define HISTORY_FILE "history"
#define LOGOUT_FILE  "logout"
#define TEMP_FILE    "/t/shell"

#define DEBUG if (debugging) putmessage
#define isinteractive() TRUE
#define sysinit()
#define systidy()
#define catch()  setjmp(&home)
#define throw(c) longjmp(&home, c)

