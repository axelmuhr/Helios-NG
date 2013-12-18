
/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      b004link.c
 --
 --      Link module for B004 type boards in IBM and NEC PCs
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/



#include "inmos.h"
#include "iserver.h"


#define NULL_LINK -1

PRIVATE LINK ActiveLink = NULL_LINK;

#ifdef NEC                                                                 /*  its really a B010  */
#define C012_DEFAULT_BASE   0xd0
#define C012_IDR_OFFSET        0
#define C012_ODR_OFFSET        2
#define C012_ISR_OFFSET        4
#define C012_OSR_OFFSET        6
#define C012_RESET_OFFSET      8
#define C012_ERROR_OFFSET      8
#define C012_ANALYSE_OFFSET   10
#else
#define C012_DEFAULT_BASE  0x150
#define C012_IDR_OFFSET        0
#define C012_ODR_OFFSET        1
#define C012_ISR_OFFSET        2
#define C012_OSR_OFFSET        3
#define C012_RESET_OFFSET     16
#define C012_ERROR_OFFSET     16
#define C012_ANALYSE_OFFSET   17
#endif

unsigned int C012_base, 
	     C012_idr,
	     C012_odr,
	     C012_isr,
	     C012_osr,
	     C012_reset,
	     C012_error,
	     C012_analyse;



/*
 *   Open Link
 */

PUBLIC LINK OpenLink ( Name )
   char *Name;
{
   if ( ActiveLink != NULL_LINK )
      return( ER_LINK_CANT );

   if ( *Name == '\0' || Name == (char *) 0 )
      C012_base = C012_DEFAULT_BASE;
   else if ( sscanf( Name, "#%x", &C012_base ) != 1 &&
             sscanf( Name, "%x", &C012_base ) != 1)
         return(ER_LINK_BAD);


   C012_idr     = C012_base + C012_IDR_OFFSET;
   C012_odr     = C012_base + C012_ODR_OFFSET;
   C012_isr     = C012_base + C012_ISR_OFFSET;
   C012_osr     = C012_base + C012_OSR_OFFSET;
   C012_reset   = C012_base + C012_RESET_OFFSET;
   C012_error   = C012_base + C012_ERROR_OFFSET;
   C012_analyse = C012_base + C012_ANALYSE_OFFSET;

   ActiveLink = 1;
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




#ifndef B4ASM

/*
 *   Read Link
 */

int ReadLink ( LinkId, Buffer, Count, Timeout )
   LINK LinkId;
   char *Buffer;
   unsigned int Count;
   int Timeout;
{
   register int n, l ;

   if ( LinkId != ActiveLink )
      return( -1 );

   if ( Count < 1 )
      return( -2 );

   for( n=0 ; Count-- ; n++ )
      {
	 for( l=0 ; ; )
	    if ( inp(C012_isr) & 1  )
	       {
#ifdef FROSTED_FLAKES
		  *Buffer = inp(C012_idr) & 0xff;
		  printf("<%02X", *Buffer++);
#else
		  *Buffer++ = inp(C012_idr) & 0xff;
#endif
		  break;
	       }
	    else
	       {
		  if ( ++l == 10000 )                                      /*  crude timout  */
		     return(n);
		  kbhit();
	       }
       }
   return( n );
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
   register int n, l ;

   if ( LinkId != ActiveLink )
      return( -1 );
   if ( Count < 1 )
      return( -2 );
    for( n=0 ; Count-- ; n++ )
       {
	  for( l=0 ; ; )
	     if ( inp(C012_osr) & 1  )
		{
#ifdef FROSTED_FLAKES
		   outp(C012_odr, (*Buffer & 0xff));
		   printf(">%02X", *Buffer++);
#else
		   outp(C012_odr, (*Buffer++ & 0xff));
#endif
		   break;
		}
	     else
		{
		   if ( ++l == 10000 )
		      return(n);
		   kbhit();
		}
       }       
   return( n );
}

#endif  /*  B4ASM  */




#pragma loop_opt(off)

/*
 *   Reset Link
 */

PUBLIC int ResetLink ( LinkId )
   LINK LinkId;
{
   int i;

   if ( LinkId != ActiveLink )
      return( -1 );

   outp( C012_analyse, 0 );                                                /*  deassert analyse  */
   for ( i = 0; i < 10000; i++ );                                          /*  wait a while      */
   outp( C012_reset, 0 );                                                  /*  deassert reset    */
   for ( i = 0; i < 10000; i++ );
   outp( C012_reset, 1 );                                                  /*  assert reset      */
   for ( i = 0; i < 10000; i++ );
   outp( C012_reset, 0 );                                                  /*  deassert reset    */
   for ( i = 0; i < 10000; i++ );
   return( 1 );
}



/*
 *   Analyse Link
 */
 
PUBLIC int AnalyseLink ( LinkId )
   LINK LinkId;
{
   int i;

   if ( LinkId != ActiveLink )
      return( -1 );

   outp( C012_analyse, 0 );                                                /*  deassert analyse  */
   for ( i = 0; i < 10000; i++ );
   outp( C012_analyse, 1 );                                                /*  assert analyse    */
   for ( i = 0; i < 10000; i++ );
   outp( C012_reset, 1 );                                                  /*  assert reset      */
   for ( i = 0; i < 10000; i++ );
   outp( C012_reset, 0 );                                                  /*  deassert reset    */
   for ( i = 0; i < 10000; i++ );
   outp(C012_analyse, 0 );                                                 /*  deassert analyse  */
   for (i = 0; i < 10000; i++);
   return( 1 );
}

#pragma loop_opt(on)




/*
 *   Test Error
 */
 
PUBLIC int TestError ( LinkId )
   LINK LinkId;
{
   if ( LinkId != ActiveLink )
      return( -1 );

   return ( ( inp(C012_error) & 1 ) ? 0 : 1 ) ;
}




/*
 *   Test Read
 */

PUBLIC int TestRead ( LinkId )
   LINK LinkId;
{
   if ( LinkId != ActiveLink )
      return( -1 );

   return ( ( inp(C012_isr) & 1 ) ? 0 : 1 ) ;
}




/*
 *   Test Write
 */

PUBLIC int TestWrite ( LinkId )
   LINK LinkId;
{
   if ( LinkId != ActiveLink )
      return( -1 );

   return ( ( inp(C012_osr) & 1 ) ? 0 : 1 ) ;
}



/*
 *   Eof
 */


