/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      sun386_b008link.c
 --
 --      Link module for B008 boards in Sun 386i
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --      
 --	 Modified for Sun 386i by Chris Farey, K-par Systems Ltd.
 --
 --   ---------------------------------------------------------------------------
*/



#include "inmos.h"
#include "iserver.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/ims_b008cmd.h>

#define NULL_LINK -1

PRIVATE LINK ActiveLink = NULL_LINK;

PRIVATE BYTE DefaultDevice[64] = "/dev/bviii0";

PRIVATE int TheCurrentTimeout = -1;




/*
 *   Open Link
 */

PUBLIC LINK OpenLink ( Name )
   BYTE *Name;
{
   if ( ActiveLink != NULL_LINK )
      return( ER_LINK_CANT );
   
   if ( (*Name == 0) || (Name == NULL) )
      strcpy( Name, DefaultDevice );
   
   if( ( ActiveLink = open( Name, O_RDWR ) ) < 0 )
      return( ER_LINK_CANT );
   return( ActiveLink );
}




/*
 *   Close Link
 */

PUBLIC int CloseLink ( LinkId )
   LINK LinkId;
{
   if ( LinkId != ActiveLink )
      return( -1 );

   close( ActiveLink );
   ActiveLink = NULL_LINK;
   return( 1 );
}




/*
 *   Read Link
 */
 
PUBLIC int ReadLink ( LinkId, Buffer, Count, Timeout )
   LINK LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
   if ( LinkId != ActiveLink )
      return( -1 );
   if ( Count < 1 )
      return( -2 );

  if( Timeout && ( Timeout != TheCurrentTimeout) )
     {
        union B008_IO io;
        int flag;

        io.set.op = SETTIMEOUT;
        io.set.val = Timeout;
        if ( ioctl(LinkId, SETFLAGS, &io, flag) == -1 ) return -1;
        TheCurrentTimeout = Timeout;
     }
  return( read( LinkId, Buffer, Count ) );
}   




/*
 *   Write Link
 */
 
PUBLIC int WriteLink ( LinkId, Buffer, Count, Timeout )
   LINK LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
   if ( LinkId != ActiveLink )
      return( -1 );
   if ( Count < 1 )
      return( -2 );

  if( Timeout && ( Timeout != TheCurrentTimeout) )
     {
        union B008_IO io;
        int flag;

        io.set.op = SETTIMEOUT;
        io.set.val = Timeout;
        if ( ioctl(LinkId, SETFLAGS, &io, flag) == -1 ) return -1;
        TheCurrentTimeout = Timeout;
     }

  return( write( LinkId, Buffer, Count ) );
}




/*
 *   Reset Link
 */
 
PUBLIC int ResetLink ( LinkId )
   LINK LinkId;
{
   int flag;
   union B008_IO io;
   
   if ( LinkId != ActiveLink)
      return( -1 );

   io.set.op = RESET;
   if ( ioctl(LinkId, SETFLAGS, &io, flag) == -1 ) return -1;
   return( 1 );
}




/*
 *   Analyse Link
 */
 
PUBLIC int AnalyseLink ( LinkId )
   LINK LinkId;
{
   int flag;
   union B008_IO io;

   if ( LinkId != ActiveLink )
      return( -1 );
  
   io.set.op = ANALYSE;
   
   if ( ioctl(LinkId, SETFLAGS, &io, flag) == -1 ) return -1;
   return( 1 );
} 




/*
 *   Test Error
 */
 
PUBLIC int TestError ( LinkId )
   LINK LinkId;
{
   union B008_IO io;
   int flag;

   if ( LinkId != ActiveLink )
      return( -1 );
 
   if ( ioctl(LinkId, READFLAGS, &io, flag) == -1 ) return -1;
   return ((int) io.status.error_f ) ;
}  




/*
 *   Test Read
 */
 
PUBLIC int TestRead ( LinkId )
   LINK LinkId;
{
   union B008_IO io;
   int flag;

   if ( LinkId != ActiveLink )
      return( -1 );
 
   if ( ioctl(LinkId, READFLAGS, &io, flag) == -1 ) return -1;
   return ((int) io.status.read_f ) ;
}




/*
 *   Test Write
 */
 
PUBLIC int TestWrite ( LinkId )
   LINK LinkId;
{
   union B008_IO io;
   int flag;

   if ( LinkId != ActiveLink )
      return( -1 );
 
   if ( ioctl(LinkId, READFLAGS, &io, flag) == -1 ) return -1;
   return ((int) io.status.write_f ) ;
}



/*
 *   Set DMA mode
 */
 
PUBLIC int SetDma( LinkId, Mode )
   LINK LinkId;
   char Mode;
{
   union B008_IO io;
   int flag;

   if ( LinkId != ActiveLink )
      return( -1 );
 
   if ( Mode == 'r' )
       io.set.op = SETREADDMA;
    else if ( Mode == 'w' )
       io.set.op = SETWRITEDMA;
    else if ( Mode == 'n' )
       io.set.op = RESETDMA;
    else
	return( -1 );
   
   if ( ioctl(LinkId, SETFLAGS, &io, flag) == -1 ) return -1;
   return ( 1 );
}


/*
 *  Eof
 */
