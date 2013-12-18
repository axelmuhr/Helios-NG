
/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      b014link.c
 --
 --      Link module for B014 boards with S502 device driver
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/



#include <stdio.h>
#include <fcntl.h>
#include <sys/ims_bcmd.h>

#include "inmos.h"
#include "iserver.h"

#define NULL_LINK -1

PRIVATE LINK ActiveLink = NULL_LINK;

PRIVATE BYTE DefaultDevice[64] = "/dev/bxiv0";

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
        union B014_IO io;
        int flag;

        io.set.op = SETTIMEOUT;
        io.set.val = Timeout;
        ioctl(LinkId, SETFLAGS, &io, flag);
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
        union B014_IO io;
        int flag;

        io.set.op = SETTIMEOUT;
        io.set.val = Timeout;
        ioctl(LinkId, SETFLAGS, &io, flag);
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
   union B014_IO io;
   
   if ( LinkId != ActiveLink)
      return( -1 );

   io.set.op = RESET;
   ioctl(LinkId, SETFLAGS, &io, flag);
   return( 1 );
}




/*
 *   Analyse Link
 */
 
PUBLIC int AnalyseLink ( LinkId )
   LINK LinkId;
{
   int flag;
   union B014_IO io;

   if ( LinkId != ActiveLink )
      return( -1 );
  
   io.set.op = ANALYSE;
   
   ioctl(LinkId, SETFLAGS, &io, flag);
   return( TRUE );
} 




/*
 *   Test Error
 */
 
PUBLIC int TestError ( LinkId )
   LINK LinkId;
{
   union B014_IO io;
   int flag;

   if ( LinkId != ActiveLink )
      return( -1 );
 
   ioctl(LinkId, READFLAGS, &io, flag);
   return ((int) io.status.error_f ) ;
}  




/*
 *   Test Read
 */
 
PUBLIC int TestRead ( LinkId )
   LINK LinkId;
{
   union B014_IO io;
   int flag;

   if ( LinkId != ActiveLink )
      return( -1 );
 
   ioctl(LinkId, READFLAGS, &io, flag);
   return ((int) io.status.read_f ) ;
}




/*
 *   Test Write
 */
 
PUBLIC int TestWrite ( LinkId )
   LINK LinkId;
{
   union B014_IO io;
   int flag;

   if ( LinkId != ActiveLink )
      return( -1 );
 
   ioctl(LinkId, READFLAGS, &io, flag);
   return ((int) io.status.write_f ) ;
}



/*
 *  Eof
 */
