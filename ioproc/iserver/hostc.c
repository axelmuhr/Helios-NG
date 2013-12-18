
/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      hostc.c
 --
 --      Primary environment operations
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/



#include <stdio.h>
#include <string.h>
#include <time.h>

#if (BOARD_ID == UDP)
#include "udplink.h"
#endif

#ifdef SUN
#include <sys/termios.h>
#endif

#ifdef VMS
#include <ssdef.h>
#include <iodef.h>
#include <descrip.h>
#include <ssdef.h>
#include <psldef.h>
#endif

#ifdef HELIOS
#include <nonansi.h>
#include <stdlib.h>
#include <attrib.h>
#endif

#include "inmos.h"
#include "iserver.h"
#include "pack.h"


EXTERN BOOL CocoPops;                                                      /*  for DEBUG  */
EXTERN BOOL VerboseSwitch;

EXTERN BYTE Tbuf[TRANSACTION_BUFFER_SIZE];

EXTERN int TheLink;

PRIVATE BYTE DataBuffer[MAX_SLICE_LENGTH+1];
PRIVATE int Size;

#define ORG_MODE 0
#define GET_MODE 1
#define POLL_MODE 2

PRIVATE BOOL TermMode = ORG_MODE;

#ifdef SUN
PRIVATE struct termios OrgMode, CurMode;
#endif

#ifdef HELIOS
PRIVATE Attributes CurAttributes, OrgAttributes;
PRIVATE Stream *InputStream;
#endif

#ifdef VMS       
PRIVATE short int InputChan;                                               /*  declare channel  */
$DESCRIPTOR( InputDescriptor, "TT:" );                                     /*  and descriptor for input  */

PRIVATE struct IOSB_DESC {
			    short int Status;
			    short int Count;
			    int DeviceInfo;
			  } iosb_desc;
#endif




PUBLIC VOID HostEnd()
{
#ifdef SUN
   ioctl(0,TCSETS,&OrgMode);
#endif
#ifdef HELIOS
   SetAttributes( InputStream, &CurAttributes );
#endif
}


PUBLIC VOID ResetTerminal()
{
#ifdef SUN
   if ( TermMode != ORG_MODE )
      {
	 ioctl(0, TCSETS, &OrgMode);
	 TermMode = ORG_MODE;
      }
#endif
#ifdef HELIOS
   if ( TermMode != ORG_MODE )
      {
	 SetAttributes( InputStream, &OrgAttributes );
	 TermMode = ORG_MODE;
      }
#endif
}


PUBLIC VOID HostBegin()
{
#ifdef SUN
   ioctl(0,TCGETS,&OrgMode);
   ioctl(0,TCGETS,&CurMode);
#endif
#ifdef HELIOS
   InputStream = fdstream(0);
   GetAttributes( InputStream, &OrgAttributes );
   GetAttributes( InputStream, &CurAttributes );
#endif
#ifdef VMS
   if ( sys$assign(&InputDescriptor, &InputChan, PSL$C_USER, 0 ) != SS$_NORMAL )
      {
	 DEBUG((SE, "cannot sys$assign TT:"));
      }
#endif

}




/*
 *   GetAKey
 */

PUBLIC BYTE GetAKey()
{
   BYTE c;

#ifdef SUN
   if ( TermMode == ORG_MODE )
      {
	 CurMode.c_iflag &= ~ICRNL;
	 CurMode.c_lflag &= ~(ICANON | ECHO);
	 CurMode.c_cc[VTIME] = 0;
	 CurMode.c_cc[VMIN] = 1;
	 ioctl( 0, TCSETS, &CurMode );
	 TermMode = GET_MODE;
      }
   else
      if ( TermMode == POLL_MODE )
	 {
	 CurMode.c_cc[VTIME] = 0;
	 CurMode.c_cc[VMIN] = 1;
	 ioctl( 0, TCSETS, &CurMode );
	 TermMode = GET_MODE;
	 }
   (void)read(0, &c, 1);
#endif
#ifdef MSC
   c = getch();
#endif
#ifdef HELIOS
   if ( TermMode == ORG_MODE )
      {
	 RemoveAttribute(&CurAttributes,ConsoleEcho);
	 AddAttribute(&CurAttributes,ConsoleRawInput);
	 SetAttributes(InputStream,&CurAttributes);
	 TermMode = GET_MODE;
      }
   (void)Read(InputStream, &c, 1, -1);
#endif
#ifdef VMS
    (void)SYS$QIOW(0, InputChan, (IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR ), &iosb_desc, 0, 0, &c, 1, 0, 0L, 0, 0);
#endif

   return(c);
}




/*
 *   SpGetKey
 */

PUBLIC VOID SpGetkey()
{
#if (BOARD_ID == UDP)
/* the udp version of iserver requires that the link is kept active,
   if it is inactive for X seconds the transputer will reset itself.
   Therefore this function polls the keyboard & sends an ACTIVE frame
   every X seconds to keep the transputer alive. */
   BUFFER_DECLARATIONS;
   BYTE c;
   int timeval1,timeval2,finished;

#ifdef VMS
   struct CHARACTERISTICS {
			     short  Count;
			     char   ch;
			     char   res1;
			     long   res2;
			   } Chars;
#endif

   DEBUG(( "SP.GETKEY {udp}" ));
   INIT_BUFFERS;           
   finished = FALSE;  
   time (&timeval1);

   while (!finished) {
     time (&timeval2);
     /* if 10 seconds elapsed, check link still active */
     if ((timeval2-timeval1) >= QUIETTIMEOUT) {
       if (TestLink(TheLink) != SUCCEEDED) {
	 finished = TRUE;
	 PUT_BYTE( SP_ERROR );
	 break;
       } else {
	 time (&timeval1);
       };
     };

#ifdef MSC
     if (kbhit()) {
       c = getch();
       PUT_BYTE( SP_SUCCESS );
       PUT_BYTE( c );    
       finished = TRUE;
     };
#endif

#ifdef SUN          
     if ( TermMode == ORG_MODE ) {
       CurMode.c_iflag &= ~ICRNL;
       CurMode.c_lflag &= ~(ICANON | ECHO);
       CurMode.c_cc[VTIME] = 1;
       CurMode.c_cc[VMIN] = 0;
       ioctl( 0, TCSETS, &CurMode );
       TermMode = POLL_MODE;
      } else {
	if ( TermMode == GET_MODE ) {
	  CurMode.c_cc[VTIME] = 1;
	  CurMode.c_cc[VMIN] = 0;
	  ioctl( 0, TCSETS, &CurMode );
	  TermMode = POLL_MODE;
	};
      };
      if (read(0, &c, 1) != 0) {
	PUT_BYTE( SP_SUCCESS );
	PUT_BYTE( c );   
	finished = TRUE;
      };
#endif

#ifdef VMS
     (void)SYS$QIOW( 0, InputChan, (IO$_SENSEMODE | IO$M_TYPEAHDCNT ), &iosb_desc, 0, 0, &Chars, sizeof(struct CHARACTERISTICS), 0, 0, 0, 0 ) ;
     if (Chars.Count > 0) {
       (void)SYS$QIOW( 0, InputChan, (IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR ), &iosb_desc, 0, 0, &c, 1, 0, 0L, 0, 0 );
       PUT_BYTE( SP_SUCCESS );
       PUT_BYTE( c );   
       finished = TRUE;
     };
#endif

#ifdef HELIOS
    if ( TermMode == ORG_MODE ) {
      RemoveAttribute(&CurAttributes,ConsoleEcho);
      AddAttribute(&CurAttributes,ConsoleRawInput);
      AddAttribute(&CurAttributes,ConsoleBreakInterrupt);
      SetAttributes(InputStream,&CurAttributes);
      TermMode = POLL_MODE;
     }
     if ( Read( InputStream, &c, 1, OneSec/10 ) == 1 ) {
       PUT_BYTE( SP_SUCCESS );
       PUT_BYTE( c );  
       finished = TRUE;
     };
#endif 
   };   /* end of while */
 
   DEBUG(("key was %c",c));
   REPLY;
#else
   BUFFER_DECLARATIONS;
   BYTE c;

   DEBUG(( "SP.GETKEY {non-udp}" ));
   INIT_BUFFERS;

   c = GetAKey();

   DEBUG(("key was %c",c));
   PUT_BYTE( SP_SUCCESS );
   PUT_BYTE( c );  
   REPLY;
#endif
}

/*
 *   SpPollkey
 */

PUBLIC VOID SpPollkey()
{
   BUFFER_DECLARATIONS;
   char c;
#ifdef VMS
   struct CHARACTERISTICS {
			     short  Count;
			     char   ch;
			     char   res1;
			     long   res2;
			   } Chars;
#endif

   DEBUG(( "SP.POLLKEY" ));
   INIT_BUFFERS;

#ifdef MSC
   if ( kbhit() )
      {
	 c = getch();
	 PUT_BYTE( SP_SUCCESS );
	 PUT_BYTE( c );  
      }
   else
      {
	 PUT_BYTE( SP_ERROR );
      }
#endif
#ifdef SUN
   if ( TermMode == ORG_MODE )
      {
	 CurMode.c_iflag &= ~ICRNL;
	 CurMode.c_lflag &= ~(ICANON | ECHO);
	 CurMode.c_cc[VTIME] = 1;
	 CurMode.c_cc[VMIN] = 0;
	 ioctl( 0, TCSETS, &CurMode );
	 TermMode = POLL_MODE;
      }
   else
      if ( TermMode == GET_MODE )
	 {
	    CurMode.c_cc[VTIME] = 1;
	    CurMode.c_cc[VMIN] = 0;
	    ioctl( 0, TCSETS, &CurMode );
	    TermMode = POLL_MODE;
	 }

   if ( read(0, &c, 1) == 0 )
      {
	 PUT_BYTE( SP_ERROR );
      }
   else
      {
	 PUT_BYTE( SP_SUCCESS );
	 PUT_BYTE( c );  
      }
#endif

#ifdef VMS
   (void)SYS$QIOW( 0, InputChan, (IO$_SENSEMODE | IO$M_TYPEAHDCNT ), &iosb_desc, 0, 0, &Chars, sizeof(struct CHARACTERISTICS), 0, 0, 0, 0 ) ;
   if ( Chars.Count > 0 )
      {
	 (void)SYS$QIOW( 0, InputChan, (IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR ), &iosb_desc, 0, 0, &c, 1, 0, 0L, 0, 0 );
	 PUT_BYTE( SP_SUCCESS );
	 PUT_BYTE( c );  
      }
   else
      {
	 PUT_BYTE( SP_ERROR );
      }
#endif
#ifdef HELIOS
   if ( TermMode == ORG_MODE )
      {
	 RemoveAttribute(&CurAttributes,ConsoleEcho);
	 AddAttribute(&CurAttributes,ConsoleRawInput);
	 AddAttribute(&CurAttributes,ConsoleBreakInterrupt);
	 SetAttributes(InputStream,&CurAttributes);
	 TermMode = POLL_MODE;
      }
   if ( Read( InputStream, &c, 1, OneSec/10 ) == 1 )
      {
	 PUT_BYTE( SP_SUCCESS );
	 PUT_BYTE( c );  
      }
   else
      {
	 PUT_BYTE( SP_ERROR );
      }
#endif
   REPLY;

}




/*
 *   SpGetenv
 */

PUBLIC VOID SpGetenv()
{
   BUFFER_DECLARATIONS;
   BYTE *Name;

   DEBUG(( "SP.GETENV" ));
   INIT_BUFFERS;

   Name = &DataBuffer[0];
   GET_SLICE( Size, Name ); *(Name+Size)=0; DEBUG(( "\"%s\"", Name ));

   if( *Name == 0 )
      {
	 PUT_BYTE( SP_ERROR );
      }
   else
      {
	 if( ( Name=(BYTE *)getenv( Name ) ) == NULL )
	    {
	       PUT_BYTE( SP_ERROR );
	    }
	 else
	    {
	       DEBUG(( "\"%s\"", Name ));
	       PUT_BYTE( SP_SUCCESS );
	       Size = strlen( Name );
	       PUT_SLICE( Size, Name );
	    }
      }
   REPLY;
}



/*
 *   SpTime
 */

PUBLIC VOID SpTime()
{
   BUFFER_DECLARATIONS;
   long Time, UTCTime;

   DEBUG(( "SP.TIME" ));
   INIT_BUFFERS;

#ifdef MSC   
   tzset();
   time( &UTCTime );
   Time = UTCTime - timezone;
   PUT_BYTE( SP_SUCCESS );
   PUT_INT32( Time );
   PUT_INT32( UTCTime );
   REPLY;
#endif

#ifdef SUN
   UTCTime = time(NULL);
   Time = UTCTime + (localtime(&UTCTime))->tm_gmtoff;
   PUT_BYTE( SP_SUCCESS );
   PUT_INT32( Time );
   PUT_INT32( UTCTime );
   REPLY;
#endif

#ifdef HELIOS
   time( &Time );
   UTCTime = 0L;
   PUT_BYTE( SP_SUCCESS );
   PUT_INT32( Time );
   PUT_INT32( UTCTime );
   REPLY;
#endif

#ifdef VMS
   time( &Time );      
   UTCTime = 0L;
   PUT_BYTE( SP_SUCCESS );
   PUT_INT32( Time );
   PUT_INT32( UTCTime );
   REPLY;
#endif

}




/*
 *   SpSystem
 */

PUBLIC VOID SpSystem()
{
   BUFFER_DECLARATIONS;
   BYTE *Command;
   INT32 Status;

   DEBUG(( "SP.SYSTEM" ));
   INIT_BUFFERS;

#ifndef UNKNOWN_HOST
   Command = &DataBuffer[0];
   GET_SLICE( Size, Command ); *(Command+Size)=0; DEBUG(( "\"%s\"", Command ));
   Status = system( Command );
   DEBUG(( "status %ld", Status ));
   PUT_BYTE( SP_SUCCESS );
   PUT_INT32( Status );
   REPLY;
#else
   PUT_BYTE( SP_UNIMPLEMENTED );
   REPLY;
#endif

}




/*
 *   SpExit
 */

PUBLIC int SpExit()
{
   BUFFER_DECLARATIONS;
   long Status;
   
   DEBUG(( "SP.EXIT" ));
   INIT_BUFFERS;
   
   GET_INT32( Status );
   DEBUG(( "%ld", Status ));

   if( Status == 999999999 )
      Status = TERMINATE_OK_EXIT;
   else if( Status == -999999999 )
      Status = TERMINATE_FAIL_EXIT;
   else
      Status = TERMINATE_OTHER_STATUS;

   DEBUG(( "exit with %d", (int)Status ));

   PUT_BYTE( SP_SUCCESS );
   REPLY ( (int)Status );
}



/*
 *   Eof
 */

