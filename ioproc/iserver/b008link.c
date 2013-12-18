
/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      b008link.c
 --
 --      Link module for B008 with PC device driver S701
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/



#include <stdio.h>
#include <dos.h>
#include <fcntl.h>

#include "inmos.h"
#include "iserver.h"


#define B008_RESET      (0)
#define B008_ANALYSE    (1)
#define B008_TIMEOUT    (2)

#define SET_FLAGS 0x03
#define READ_FLAGS 0x02

#define BINARY (32)
#define ISDEV (128)

struct B008_IO { int io_val; int io_op; };


#define DEFAULT_LINK "LINK1"
#define NULL_LINK -1

PRIVATE LINK ActiveLink = NULL_LINK;
PRIVATE int CurrentTimeout = 0;



PRIVATE VOID ioctl ( Fd, Type, Func )
   int Fd, Type;
   struct B008_IO *Func;
{
   union REGS regs;
   
   regs.h.ah = 0x44;
   regs.h.al = (unsigned char)Type;
   regs.x.cx = sizeof(struct B008_IO );
   regs.x.bx = Fd;      
   regs.x.dx = (unsigned int)Func;
   int86( 0x21, &regs, &regs );
}




/*
 *   Open Link
 */

LINK OpenLink ( Name )
   char *Name;
{
   union REGS regs;
   
   if ( ActiveLink != NULL_LINK )
      return( ER_LINK_CANT );

   if ( Name == NULL || *Name == 0 )
      Name = DEFAULT_LINK;
      
   if ( ( ActiveLink = open( Name, O_RDWR | O_BINARY ) ) < 0 )
      return( ER_LINK_CANT );
   
   regs.x.ax = 0x4400;
   regs.x.bx = ActiveLink;
   int86( 0x21, &regs, &regs );

   if( ( regs.h.dl & ISDEV ) == 0 )
      {
         fprintf( stderr, "B008: \"%s\" is not a device\n", Name );
         close( ActiveLink );
         ActiveLink = NULL_LINK;
         return( ER_LINK_CANT );
      }
   
   regs.h.dh = 0;         /* set binary mode */
   regs.h.dl |= BINARY;
   regs.x.ax = 0x4401;
   int86( 0x21, &regs, &regs );
   
   return( ActiveLink );
}




/*
 *   Close Link
 */

int CloseLink ( LinkId )
   int LinkId;
{
   if ( LinkId != ActiveLink )
      return( ER_LINK_BAD );

   close( ActiveLink );
   ActiveLink = NULL_LINK;
   return( SUCCEEDED );
}




int ReadLink ( LinkId, Buffer, Count, Timeout )
   int LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
   struct B008_IO flags;
   
   if ( LinkId != ActiveLink )
      return( ER_LINK_BAD );

   if ( Count < 1 )
      return( ER_LINK_NODATA );      

   if (Timeout != CurrentTimeout )
   {
      flags.io_op = B008_TIMEOUT;
      flags.io_val = Timeout;
      ioctl( LinkId, SET_FLAGS, &flags );
      
      CurrentTimeout = Timeout;
   }

   return( read( LinkId, Buffer, Count ) );
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
   struct B008_IO flags;

   if ( LinkId != ActiveLink )
      return( ER_LINK_BAD );

   if ( Count < 1 )
      return( ER_LINK_NODATA );

   if (Timeout != CurrentTimeout )
   {
      flags.io_op = B008_TIMEOUT;
      flags.io_val = Timeout;
      ioctl( LinkId, SET_FLAGS, &flags );
      
      CurrentTimeout = Timeout;
   }

   return( write( LinkId, Buffer, Count ) );
}



/*
 *   Reset Link
 */

int ResetLink ( LinkId )
   int LinkId;
{
   struct B008_IO flags;

   if ( LinkId != ActiveLink )
      return( ER_LINK_BAD );

   flags.io_op = B008_RESET;
   ioctl( LinkId, SET_FLAGS, &flags );
   return( SUCCEEDED );
}




/*
 *   Analyse Link
 */

int AnalyseLink ( LinkId )
   int LinkId;
{
   struct B008_IO flags;

   if ( LinkId != ActiveLink )
      return( ER_LINK_BAD );

   flags.io_op = B008_ANALYSE;
   ioctl( LinkId, SET_FLAGS, &flags );
   return( SUCCEEDED );
}




/*
 *   Test Error
 */

int TestError ( LinkId )
   int LinkId;
{
   struct B008_IO flags;

   if ( LinkId != ActiveLink )
      return( -1 );

   ioctl( LinkId, READ_FLAGS, &flags );
   return ( flags.io_val & 0x01 ) ;
}




/*
 *   Test Read
 */

int TestRead ( LinkId )
   int LinkId;
{
   struct B008_IO flags;

   if ( LinkId != ActiveLink )
      return( ER_LINK_BAD );

   ioctl( LinkId, READ_FLAGS, &flags );
   return ( flags.io_val & 0x08 ) ;
}



/*
 *   Test Write
 */

int TestWrite ( LinkId )
   int LinkId;
{
   struct B008_IO flags;

   if ( LinkId != ActiveLink )
      return( ER_LINK_BAD );

   ioctl( LinkId, READ_FLAGS, &flags );
   return ( flags.io_val & 0x04 ) ;
}



/*
 *   Eof
 */

