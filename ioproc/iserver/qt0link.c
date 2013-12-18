
/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      qt0link.c
 --
 --      Link module for Caplin QT0 boards (v1.? driver and v2.0 proms)
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/


#ifdef VMS

#include <descrip.h>
#include <ssdef.h>
#include <iodef.h>
#include <stdio.h>

#include "inmos.h"
#include "iserver.h"


#define ASSERT_RESET 0x0004
#define ASSERT_ANALYSE 0x0002
#define DEASSERT_ALL 0x0000

#define DMA_THRESHOLD 2


char LinkName[128]="sys$transputer";


PRIVATE unsigned short chan;
struct dsc$descriptor_s _Link, *Link = &_Link;


PRIVATE struct io_block {
                           short status;
                           short count;
                           int qt0_regs;
                        } Io;



/*
 *   Open Link
 */

int OpenLink ( Name )
   char *Name;
{
   int status;

   if ( (Name != NULL) && (*Name) )
      strcpy( &LinkName[0], Name );

   Link->dsc$w_length = strlen(&LinkName[0]);
   Link->dsc$b_dtype = DSC$K_DTYPE_T;
   Link->dsc$b_class = DSC$K_CLASS_S;
   Link->dsc$a_pointer = &LinkName[0];

   status = sys$alloc(Link,0,0,0,0);
   if ( status != SS$_NORMAL && status != SS$_DEVALRALLOC )
      return(ER_LINK_CANT);
   if ( sys$assign(Link, &chan, 0, 0) != SS$_NORMAL )
      return(ER_LINK_CANT);
   return(1);
}




/*
 *   Close Link
 */

int CloseLink ( LinkId )
    int LinkId;
{
   if ( sys$dalloc(Link,0) != SS$_NORMAL )
      return(-1);
   return(1);
}




/*
 *   Read Link
 */

int ReadLink ( LinkId, Buffer, Count, Timeout )
   int LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
   int QtTimeout;

   /*  Caplin only do 1 second timouts,  however this is a dire way to use a VAX anyway
       this module needs to be rewritten to use ASTs (when Caplin allow ASTs on error set)  */

   QtTimeout = 5;

   if ( Count > DMA_THRESHOLD )
      (void)sys$qiow( 0, chan, (IO$_READVBLK | IO$M_TIMED), &Io, 0, 0, Buffer, Count, QtTimeout, 0,0,0) ;
   else
      (void) sys$qiow( 0, chan, (IO$_READVBLK | IO$M_WORD | IO$M_TIMED), &Io, 0, 0, Buffer, Count, QtTimeout, 0,0,0);
   return(Io.count);
}




/*
 *   Write Link
 */

int WriteLink ( LinkId, Buffer, Count, Timeout )
   int LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
   int QtTimeout;

/* QtTimeout = (Timeout+10)/10; */

   QtTimeout = 5;							   /*  see ReadLink  */

   if ( Count > DMA_THRESHOLD )
      (void)sys$qiow( 0, chan, (IO$_WRITEVBLK | IO$M_TIMED), &Io, 0, 0, Buffer, Count, QtTimeout, 0,0,0);
   else
      (void)sys$qiow( 0, chan, (IO$_WRITEVBLK | IO$M_WORD | IO$M_TIMED), &Io, 0, 0, Buffer, Count, QtTimeout, 0,0,0);
   return(Io.count);
}




/*
 *   Reset Link
 */

int ResetLink ( LinkId )
   int LinkId;
{
   (void)sys$qiow(0, chan, IO$_SETMODE, &Io, 0, 0, 4, 0, 0, 0, 0, 0);
   if ( Io.status != SS$_NORMAL )
      return(-1);
   (void)sys$qiow(0, chan, IO$_SETMODE, &Io, 0, 0, 0, 0, 0, 0, 0, 0);
   if ( Io.status != SS$_NORMAL )
      return(-2);
   return(1);
}




/*
 *   Analyse Link
 */

int AnalyseLink ( LinkId )
   int LinkId;
{
   (void)sys$qiow(0, chan, IO$_SETMODE, &Io, 0, 0, 2, 0, 0, 0, 0, 0);
   if ( Io.status != SS$_NORMAL )
      return(-1);
   (void)sys$qiow(0, chan, IO$_SETMODE, &Io, 0, 0, 6, 0, 0, 0, 0, 0);
   if ( Io.status != SS$_NORMAL )
      return(-2);
   (void)sys$qiow(0, chan, IO$_SETMODE, &Io, 0, 0, 2, 0, 0, 0, 0, 0);
   if ( Io.status != SS$_NORMAL )
      return(-3);
   (void)sys$qiow(0, chan, IO$_SETMODE, &Io, 0, 0, 0, 0, 0, 0, 0, 0);
   if ( Io.status != SS$_NORMAL )
      return(-3);
   return(1);
}




/*
 *   Test Error
 */

int TestError ( LinkId )
   int LinkId;
{
   (void)sys$qiow(0, chan, IO$_SENSEMODE, &Io, 0, 0, 0, 0, 0, 0, 0, 0);
   if ( Io.status != SS$_NORMAL )
      return(-1);
   if ( Io.qt0_regs & 0x00010000 )
      return(0);
   else
      return(1);
}




/*
 *   Test Read
 */

int TestRead ( LinkId )
   int LinkId;
{
   return(-1);
}




/*
 *   Test Write
 */

int TestWrite ( LinkId )
   int LinkId;
{
   return(-1);
}



#endif  /*  VMS  */



/*
 *  Eof
 */
