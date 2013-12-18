/**
*
* Title:  CSH - Header file
*
* Author: Andy England
*
* $Header: /hsrc/cmds/shell/RCS/shell.h,v 1.5 1993/05/07 16:32:05 nickc Exp $
*
**/
#ifdef HELIOS
#include "_helios.h"
#endif
#ifdef UNIX
#include "unix.h"
#endif
#ifdef ATARI
#include "atari.h"
#endif
#ifdef AMIGA
#include "amiga.h"
#endif
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <pwd.h>
#include "define.h"
#include "typedef.h"
#include "prototype.h"

#ifdef TEST_FDS
#define open myopen
#define close myclose
#define dup mydup
#define pipe mypipe
#endif


extern int strnequ( char *, char *, int );
extern int strequ( char *, char * );
