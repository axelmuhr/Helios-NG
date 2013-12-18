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
 |  proccnt.c								   |
 |                                                                         |
 |    Process counter.                                                     |
 |                                                                         |
 |-------------------------------------------------------------------------|
 |                                                                         |
 |  History:                                                               |
 |    1 - O.Imbusch - 26 April 1991 - Basic version                        |
 |                                                                         |
[]-------------------------------------------------------------------------[]
                                                                                */

#include <nonansi.h>
#include <sem.h>

#include "misc.h"

#define  FLDEBUG 1
#include "error.h"

#define  PROCCNT 1
#include "proccnt.h"

TCounter Counter;

void ConstPC (void)
{
  int I;
  
  InitSemaphore (&Counter.S, 1);
  Wait (&Counter.S);
  for (I = 0; I < NOF_COUNTER; ++I)
    Counter.Cnt [I] = 0;
  Signal (&Counter.S);
}

void IncPC (int PCNr)
{
  if ((PCNr <0) || (PCNr > (NOF_COUNTER - 1)))
    Fatal (1, "Illegal index");
  else
  {
    Wait (&Counter.S);
    ++Counter.Cnt [PCNr];
    Signal (&Counter.S);
  }
}

void DecPC (int PCNr)
{
  if ((PCNr <0) || (PCNr > (NOF_COUNTER - 1)))
    Fatal (1, "Illegal index");
  else
  {
    Wait (&Counter.S);
    --Counter.Cnt [PCNr];
    Signal (&Counter.S);
  }
}

void ReportPC (void)
{
  int I,
      Buff [NOF_COUNTER],
      CntPerLine = 5;
      
  Wait (&Counter.S);
  for (I = 0; I < NOF_COUNTER; ++I)
    Buff [I] = Counter.Cnt [I];
  Signal (&Counter.S);
  
  Serious ("Process counter:");
  Serious ("\t    |\t0\t1\t2\t3\t4");
  for (I = 0; I < (RoundTo (NOF_COUNTER, CntPerLine) / CntPerLine); ++I)
  {
    if ((I % CntPerLine) == 0)
      Serious ("------------------------------------------------");
    Serious ("%d \t    |\t%d\t%d\t%d\t%d\t%d",
             I * CntPerLine,
             Buff [(I * CntPerLine) +  0],
             Buff [(I * CntPerLine) +  1],
             Buff [(I * CntPerLine) +  2],
             Buff [(I * CntPerLine) +  3],
             Buff [(I * CntPerLine) +  4]);
  }             
}
 
void DestPC (void)
{
}

#if TEST

int main (int   ArgC,
	  char *ArgV [])
{

/*
  ConstPC ();
  ReportPC ();
  IncPC (11);
  IncPC (13);
  IncPC (13);
  ReportPC ();
  DestPC ();
*/
}

#endif

/******************************************************************************
**
**  proccnt.c
**
******************************************************************************/
