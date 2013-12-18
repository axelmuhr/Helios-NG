                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                                Utilities                                |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  proccnt.h								     |
   |                                                                         |
   |    Definitions/prototypes for proccnt.c.                                |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - O.Imbusch - 26 April 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#ifndef _PROC_CNT_H
#define _PROC_CNT_H

#include "error.h"

#ifndef PROCCNT
#define PROCCNT 0
#endif

#if PROCCNT

#include <sem.h>

#define  NOF_COUNTER  100

typedef struct
        {
          int       Cnt [NOF_COUNTER];
          Semaphore S;
        } TCounter;

extern TCounter Counter;

extern void     ConstPC  (void);
extern void     IncPC    (int  PCNr);
extern void     DecPC    (int  PCNr);
extern void     ReportPC (void);
extern void     DestPC   (void);

#else

#define ConstPC  DoNothing
#define IncPC    DoNothing
#define DecPC    DoNothing
#define ReportPC DoNothing
#define DestPC   DoNothing

#endif

#endif
