
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

/* RcsId: $Id: b014.c,v 1.4 1994/06/29 13:46:19 tony Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.       			*/

/**
*** Modified to compile for the Helios I/O Server, BLV, 21.3.90. This
*** involved the following changes:
*** 1) got rid of ActiveLink, it does not make sense if multiple links are
***    supported
*** 2) renamed OpenLink() to b011_OpenLink(), etc.
*** 3) added a b011_init_link() routine.
*** 4) changed the headers to Helios.h, and took some #defines from the
***    iserver headers
**/

#if 0
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <pwd.h>
#include <sys/ims_bcmd.h>

#include "inmos.h"
#include "iserver.h"

#else

#include <sys/ioccom.h>

#include "../helios.h"
#include "ims_bcmd.h"
#include <sys/mman.h>
#define LINK int
#define EXTERN extern
#define BOOL int
#define VOID void
#define MISC_EXIT             (4)
#define ER_LINK_BAD    (-1)          /*  Failure codes for LinkIO functions */
#define ER_LINK_CANT   (-2)
#define ER_LINK_SOFT   (-3)
#define ER_LINK_NODATA (-4)
#define SUCCEEDED      (1)   
#define ABORT(a,b) _exit(a)
#endif

/**
*** There may be up to ten link adapters, labelled /dev/bxiv0 to /dev/bxiv9,
*** I think.
**/
PRIVATE Trans_link b014_links[10];

void b014_init_link()
{ int i;
  number_of_links = 10;
  link_table = &(b014_links[0]);
  for (i = 0; i < number_of_links; i++)
   sprintf(b014_links[i].link_name, "/dev/bxiv%d", i);

  for ( ; number_of_links >= 0; number_of_links--)
   { struct stat buf;
     if (stat(link_table[number_of_links-1].link_name, &buf) eq 0)
      break;                   /* OK, found the last known site */
     if (errno ne ENOENT)      /* Appears to exist, but not currently usable */
      break;
   }       
}

#if 0
#define NULL_LINK -1

PRIVATE LINK ActiveLink = NULL_LINK;
#endif

PRIVATE BYTE DefaultDevice[64] = "/dev/bxiv0";

PRIVATE int TheCurrentTimeout = -1;


/*
 *   Open Link
 */

PUBLIC LINK b014_OpenLink ( Name )
   BYTE *Name;
{ int ActiveLink;

#if 0
   if ( ActiveLink != NULL_LINK )
      return( ER_LINK_CANT );
#endif 

   if ( (*Name == 0) || (Name == NULL) )
      strcpy( Name, DefaultDevice );
   
   if( ( ActiveLink = open( Name, O_RDWR ) ) < 0 )
      return( ER_LINK_CANT );
   return( ActiveLink );
}


/*
 *   Close Link
 */

PUBLIC int b014_CloseLink ( LinkId )
   LINK LinkId;
{
#if 0
   if ( LinkId != ActiveLink )
      return( -1 );
#endif

   close( LinkId);
#if 0
   ActiveLink = NULL_LINK;
#endif
   return( 1 );
}


/*
 *   Read Link
 */
 
PUBLIC int b014_ReadLink ( LinkId, Buffer, Count, Timeout )
   LINK LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
#if 0
   if ( LinkId != ActiveLink )
      return( -1 );
#endif
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
 
PUBLIC int b014_WriteLink ( LinkId, Buffer, Count, Timeout )
   LINK LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
#if 0
   if ( LinkId != ActiveLink )
      return( -1 );
#endif
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
 
PUBLIC int b014_ResetLink ( LinkId )
   LINK LinkId;
{
   int flag;
   union B014_IO io;

#if 0   
   if ( LinkId != ActiveLink)
      return( -1 );
#endif

   io.set.op = RESET;
   ioctl(LinkId, SETFLAGS, &io, flag);
   return( 1 );
}


/*
 *   Analyse Link
 */
 
PUBLIC int b014_AnalyseLink ( LinkId )
   LINK LinkId;
{
   int flag;
   union B014_IO io;

#if 0
   if ( LinkId != ActiveLink )
      return( -1 );
#endif 

   io.set.op = ANALYSE;
   
   ioctl(LinkId, SETFLAGS, &io, flag);
   return( TRUE );
} 


/*
 *   Test Error
 */
 
PUBLIC int b014_TestError ( LinkId )
   LINK LinkId;
{
   union B014_IO io;
   int flag;

#if 0
   if ( LinkId != ActiveLink )
      return( -1 );
#endif
 
   ioctl(LinkId, READFLAGS, &io, flag);
   return ((int) io.status.error_f ) ;
}  


/*
 *   Test Read
 */
 
PUBLIC int b014_TestRead ( LinkId )
   LINK LinkId;
{
   union B014_IO io;
   int flag;

#if 0
   if ( LinkId != ActiveLink )
      return( -1 );
#endif
 
   ioctl(LinkId, READFLAGS, &io, flag);
   return ((int) io.status.read_f ) ;
}


/*
 *   Test Write
 */
 
PUBLIC int b014_TestWrite ( LinkId )
   LINK LinkId;
{
   union B014_IO io;
   int flag;

#if 0
   if ( LinkId != ActiveLink )
      return( -1 );
#endif
 
   ioctl(LinkId, READFLAGS, &io, flag);
   return ((int) io.status.write_f ) ;
}



/*
 *  Eof
 */
