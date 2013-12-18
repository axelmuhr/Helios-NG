/**
*
* Title:  Helios Shell - Header File.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/_helios.h,v 1.4 1992/09/17 15:18:41 martyn Exp $
*
**/

#ifndef __shell_helios_h
#define __shell_helios_h

#define __system_io
#define errno errno
typedef int FILEHANDLE;
typedef int sysbase;
#include <syslib.h>
#include <gsp.h>
#include <stdlib.h>
#include <string.h>
#include <nonansi.h>
#include <posix.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dirent.h>
#include <nonansi.h>
#include <stdio.h>

#undef FALSE
#undef TRUE

typedef struct List LIST;
typedef struct Node NODE;
typedef struct dirent DIRENT;

typedef enum
{
  FALSE,
  TRUE
} BOOL;

#define ENVCHAR ':'

#define STACK_SIZE 5000

#define SHELL_CMD    "/helios/bin/shell"
#define LOGIN_CMD    "/helios/bin/login"

#define LOGIN_FILE   ".login"
#define LOGIN_FILE_V11   "loginrc"

#define CSHRC_FILE   ".cshrc"
#define CSHRC_FILE_V11   "cshrc"

#define HISTORY_FILE ".history"
#define HISTORY_FILE_V11 "history"

#define LOGOUT_FILE  ".logout"
#define LOGOUT_FILE_V11  "logout"

#define TEMP_FILE    "/fifo/shell"


#define DEBUG if (debugging) IOdebug

#include "dodebug.h"

#ifdef DEBUGGING
#undef DEBUG
#define DEBUG if (debugging) DoDebug
#endif

#define unixpath(p)
#define syspath(p)
#define isabspath(p) ((p)[0] == '/')
#define catch()  setjmp(home)
#define throw(c) longjmp(home, c)

#endif /* __shell_helios_h */
