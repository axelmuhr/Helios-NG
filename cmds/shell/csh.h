/**
*
* Title:  CSH - Header file
*
* Author: Andy England
*
* $Header: /hsrc/cmds/shell/RCS/csh.h,v 1.2 1992/06/29 15:45:54 nickc Exp $
*
**/
#ifdef __HELIOS
#include "_helios.h"
#endif
#ifdef unix
#include "unix.h"
#endif
#ifdef atari
#include "atari.h"
#endif
#ifdef amiga
#include "amiga.h"
#endif
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include "define.h"
#include "typedef.h"
#include "prototype.h"

