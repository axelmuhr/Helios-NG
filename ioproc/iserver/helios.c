
/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      helios.c
 --
 --      Link module for raw transputers attached to a helios node
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/



#include <stdio.h>
#include <root.h>

#include "inmos.h"


#define NULL_LINK -1

PRIVATE LINK ActiveLink = NULL_LINK;

#define SUBSYSTEM_RESET    (0x0000000)
#define SUBSYSTEM_ANALYSE  (SUBSYSTEM_RESET + 4)
#define SUBSYSTEM_ERROR    (SUBSYSTEM_RESET)

#define PAUSE {int i; for (i=0;i<10000;i++); }



/*
 *   Open Link
 */

PUBLIC LINK OpenLink ( Name )
   char *Name;
{
   LinkInfo *l;
   LinkConf Linkconf[4];
   int i;
   
   if ( ActiveLink != NULL_LINK )
      return( -1 );

   if ( sscanf( Name, "%x", &ActiveLink ) != 1 )
      ActiveLink = 2;

   if ( ActiveLink > 3 || ActiveLink < 0 )
      return( -2 );

   l = *(GetRoot()->Links)+ActiveLink;
   if (( l->Mode == 2 ) && ( l->State != 6) )
      {
         printf("openlink: link %d is an active helios link!\n", ActiveLink);
         return(-3);
      }
/*
 *   This is not implemented
	LinkData(i, &linkinfo[i] )) 
 *   so...
 */

   for (i=0; i<4; i++)
      {
         Linkconf[i].Flags = 0;
         Linkconf[i].Mode = 2;          /*  intelligent  */
         Linkconf[i].State = 3;
         Linkconf[i].Id = i;
      }
   Linkconf[ActiveLink].Mode = 1;       /*  dumb  */

   if ( Reconfigure(Linkconf) < 0 )
      return(-4);

   return(ActiveLink);
}




/*
 *   Close Link
 */

int CloseLink ( LinkId )
   int LinkId;
{
   if ( LinkId != ActiveLink )
      return( -1 );
   ActiveLink = NULL_LINK;
   return(TRUE);
}




/*
 *   Read Link
 */

int ReadLink ( LinkId, Buffer, Count, Timeout )
   LINK LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
   if ( LinkId != ActiveLink )
      return( -1 );

  if( LinkIn((word)Count, LinkId, (void *)Buffer, Timeout*(OneSec/10)) < 0)
      return(0);
   else
      return(Count);
}




/*
 *   Write Link
 */

int WriteLink ( LinkId, Buffer, Count, Timeout )
   LINK LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
   if ( LinkId != ActiveLink )
      return( -1 );

  if( LinkOut((word)Count, LinkId, (void *)Buffer, Timeout*(OneSec/10)) < 0)
      return(0);
   else
      return(Count);
}




/*
 *   Reset Link
 */
 
PUBLIC int ResetLink ( LinkId )
   LINK LinkId;
{
   int *m;
   
   if ( LinkId != ActiveLink )
      return( -1 );

   m = (int *)SUBSYSTEM_RESET; *m = 0 ;
   m = (int *)SUBSYSTEM_ANALYSE; *m = 0 ;
   PAUSE;
   m = (int *)SUBSYSTEM_RESET; *m = 1 ;
   PAUSE;
   m = (int *)SUBSYSTEM_RESET; *m = 0 ;
   PAUSE;

   return(TRUE);
}



/*
 *   Analyse Link
 */
 
PUBLIC int AnalyseLink ( LinkId )
   LINK LinkId;
{
   int *m;
   
   if ( LinkId != ActiveLink )
      return( -1 );

   m = (int *)SUBSYSTEM_ANALYSE; *m = 0 ;
   PAUSE;
   m = (int *)SUBSYSTEM_ANALYSE; *m = 1 ;
   PAUSE;
   m = (int *)SUBSYSTEM_RESET; *m = 1 ;
   PAUSE;
   m = (int *)SUBSYSTEM_RESET; *m = 0 ;
   PAUSE;
   m = (int *)SUBSYSTEM_ANALYSE; *m = 0 ;
   PAUSE;

   return(TRUE);
}




/*
 *   Test Error
 */
 
PUBLIC int TestError ( LinkId )
   LINK LinkId;
{
   int *m;
   
   if ( LinkId != ActiveLink )
      return( -1 );

   m = (int *)SUBSYSTEM_ERROR;
   return( *m & 1 );
}




/*
 *   Test Read
 */

PUBLIC int TestRead ( LinkId )
   LINK LinkId;
{
   if ( LinkId != ActiveLink )
      return( -1 );
   return(-1);
}




/*
 *   Test Write
 */

PUBLIC int TestWrite ( LinkId )
   LINK LinkId;
{
   if ( LinkId != ActiveLink )
      return( -1 );
   return(-1);
}



/*
 *   Eof
 */


