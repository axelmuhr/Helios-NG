                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                               Utitlities                                |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  fstring.h                                                              |
   |                                                                         |
   |    -Prototypes and definitions for fstring.c                            |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - O.Imbusch - 19 March 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#ifndef __FSTRING_H
#define __FSTRING_H

#include <syslib.h>
#include <string.h>

#define NewStr(P,S)      strcpy (P = (char *) (Malloc (strlen (S) + 1)), S)

extern char *CutLast      (char *String,
                           int   NoOfChars);
extern char *CutLeading   (char *String,
                           char *What);
extern char *CutAppending (char *String,
                           char *What);
extern char *PolyStr      (char *Dest,
                           char *Base,
                           int   NOfCopies);
extern char *Replace      (char *Source,
                           char  Old,
                           char  New);
extern char *UpCase       (char *Source);
extern char *LowCase      (char *Source);
extern char *strrstr      (char *S1,
                           char *S2);

#endif

/*******************************************************************************
**
**  fstring.h
**
*******************************************************************************/
