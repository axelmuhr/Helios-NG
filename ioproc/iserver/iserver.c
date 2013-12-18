/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      iserver.c
 --
 --      The main body
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/


#include <stdio.h>
#include <signal.h>
#include <string.h> 
#ifdef MSC
#include <stdlib.h>
#include <malloc.h>
#endif

#ifdef VMS
#include <unixio.h>
#include <file.h>
#endif

#ifdef HELIOS
#include <stdlib.h>
#include <posix.h>
#endif

#include "inmos.h"
#include "iserver.h"




EXTERN VOID SpOpen();
EXTERN VOID SpClose();
EXTERN VOID SpRead();
EXTERN VOID SpWrite();
EXTERN VOID SpGetBlock();
EXTERN VOID SpPutBlock();
EXTERN VOID SpGets();
EXTERN VOID SpPuts();
EXTERN VOID SpFlush();
EXTERN VOID SpSeek();
EXTERN VOID SpTell();
EXTERN VOID SpEof();
EXTERN VOID SpError();
EXTERN VOID SpRemove();
EXTERN VOID SpRename();

EXTERN VOID SpGetkey();
EXTERN VOID SpPollkey();
EXTERN VOID SpGetenv();
EXTERN VOID SpTime();
EXTERN VOID SpSystem();
EXTERN VOID SpCommand();

EXTERN VOID SpCore();
EXTERN VOID SpId();
EXTERN int  SpExit();
EXTERN VOID SpUnknown();

#ifdef MSC
EXTERN VOID SpMsdos();
#endif

EXTERN LINK OpenLink();
EXTERN int  CloseLink();
EXTERN int  ReadLink();
EXTERN int  WriteLink();
EXTERN int  ResetLink();
EXTERN int  AnalyseLink();
EXTERN int  TestError();
EXTERN int  TestRead();
EXTERN int  TestWrite();

EXTERN VOID HostBegin();
EXTERN VOID HostEnd();
EXTERN VOID ResetTerminal();




PUBLIC BYTE Tbuf[TRANSACTION_BUFFER_SIZE];                                 /*  buffer for all server operations  */

PUBLIC BYTE RealCommandLine     [ MAX_COMMAND_LINE_LENGTH+1 ];
PUBLIC BYTE DoctoredCommandLine [ MAX_COMMAND_LINE_LENGTH+1 ];

PUBLIC BOOL AnalyseSwitch;                                                 /*  command line switches  */
PUBLIC BOOL TestErrorSwitch;
PUBLIC BOOL VerboseSwitch;
PUBLIC BOOL LinkSwitch;
PUBLIC BOOL ResetSwitch;
PUBLIC BOOL ServeSwitch;
PUBLIC BOOL LoadSwitch;
PUBLIC BOOL CocoPops;

PRIVATE BYTE BootFileName[MAX_BOOT_FILE_LENGTH+1];

PRIVATE BYTE LinkName[MAX_LINK_NAME_LENGTH+1];  

PUBLIC LINK TheLink = -1 ;                                                         /*  the server's idea of the active link  */

PUBLIC BYTE *CoreDump;
PUBLIC INT32 CoreSize;

PRIVATE char *LinkMessages[] = {
   "Strange result from link module",
   "Transputer no longer available",         /* ER_LINK_BAD       */
   "No response from transputer",            /* ER_LINK_CANT      */
   "Comms software failure",                 /* ER_LINK_SOFT      */
   "Zero length data"                        /* ER_LINK_NODATA    */
};

PRIVATE char *HardwareNames[] = {
   "???", 
   "B004",
   "B008", 
   "B010",    
   "B011", 
   "B014", 
   "DRX11", 
   "QT0", 
   "IBM_CAT",
   "B015", 
   "B016",
   "UDP-Link"
};

/*
 *   Boot  -  copy the boot file to the link
 */

PRIVATE VOID Boot()
{
   FILE *Fd;
   BYTE Buffer[ BOOT_BUFFER_LENGTH ];
   int Length=0, Count=0;
   INT32 Size=0;
   static BYTE Flicks[]="|/-\\|/-\\" ;
   int Flick=0;

#ifdef FASTFILEVMS
   int extra_length, end_of_file;
   
   if ((Fd = open (BootFileName,O_RDONLY,0)) == -1)
     ABORT(MISC_EXIT, (SE, "cannot find boot file \"%s\"", BootFileName));
   INFO(("Booting root transputer..."));

   while ((Length = read (Fd, Buffer, BOOT_BUFFER_LENGTH)) > 0) {
     DEBUG(("%d", Length));                              
      end_of_file = FALSE;
     while (!end_of_file && (Length < BOOT_BUFFER_LENGTH)) {
       if ((extra_length = read(Fd, &Buffer[Length], (BOOT_BUFFER_LENGTH-Length))) <= 0)
         end_of_file = TRUE;
       else     
         Length = Length + extra_length;
     };
     Count = WriteLink (TheLink, Buffer, Length, BOOT_TIMEOUT);
     DEBUG(("WriteLink(TheLink, Buffer, %d, BOOT_TIMEOUT) in boot() returned %d\n", Length, Count));
     if (Count != Length)
     {
       if (Count < 0)
         ABORT(MISC_EXIT, (SE, "unable to write byte %d to the boot link because\n%s",
                                 Size, LinkMessages[0-Count]))
       else
         ABORT(MISC_EXIT, (SE, "unable to write byte %d to the boot link", Size ));
     }
     Size += Count;
     if ( isatty(1) ) {
       INFO(( "%c\b", Flicks[Flick] ));
       if ( ++Flick == 8 )
         Flick -= 8;
     };
   };
#else
   if ( ( Fd=fopen( BootFileName, "rb" ) ) == NULL )
      ABORT(MISC_EXIT, (SE, "cannot find boot file \"%s\"", BootFileName));
   INFO(("Booting root transputer..."));
   while ( ( Length = fread( Buffer, 1, BOOT_BUFFER_LENGTH, Fd ) ) > 0 )
      {
         DEBUG(("%d", Length));
         Count = WriteLink( TheLink, Buffer, Length, BOOT_TIMEOUT );
         DEBUG(("WriteLink(TheLink, Buffer, %d, BOOT_TIMEOUT) in boot() returned %d\n", Length, Count));
         if ( Count != Length )
         {
            if (Count < 0)
#ifdef MSC
               ABORT(MISC_EXIT, (SE, "unable to write byte %ld to the boot link because\n%s", 
                                             Size, LinkMessages[0-Count]))
#else
               ABORT(MISC_EXIT, (SE, "unable to write byte %d to the boot link because\n%s", 
                                             Size, LinkMessages[0-Count]))
#endif
            else
               ABORT(MISC_EXIT, (SE, "unable to write byte %d to the boot link", Size ));
         }
         Size += Count;
         if (isatty(1)) {
           INFO(( "%c\b", Flicks[Flick] ));
           if ( ++Flick == 8 )
             Flick -=8;
         };
      }
#endif

   if ( isatty(1) )
      {
         INFO(( "\r                           \r" ));
      }
   else
      {
         INFO(("ok\n"));
      }
   DEBUG(( "booted %ld bytes", Size ));
}




/*
 *   Core  -  peek the transputer memory into a buffer
 */

PRIVATE VOID Core()
#ifdef FASTPEEK
{            
  short int bytes_peeked, bytes_unpeeked, bytes_to_peek, num_bytes;
  static BYTE Flicks[]="|\\-/|\\-/";
  int Flick = 0;
  INT32 addr;  
             
  INFO(("Peeking root transputer..."));
  bytes_peeked = 0;
  bytes_unpeeked = CoreSize;

  while (bytes_unpeeked > 0) {          
    bytes_to_peek = bytes_unpeeked;
    if (bytes_to_peek > 1024) {
      bytes_to_peek = 1024;
    };                                          
    addr = (INT32) (bytes_peeked + 0x80000000L);
             
    num_bytes = PeekLink (TheLink, (char *) &CoreDump[bytes_peeked],
                          addr, (bytes_to_peek / 4)); /* in words */
    if (num_bytes < 0) {
      ABORT(MISC_EXIT, (SE,"timed out peeking byte %d because\n%s", 
                              bytes_peeked, LinkMessages[0-num_bytes]));
    };
    bytes_peeked = bytes_peeked + num_bytes;
    bytes_unpeeked = bytes_unpeeked - num_bytes;
    if (isatty(1)) {
      INFO(("%c\b", Flicks[Flick]));
      if (++Flick == 8) 
         Flick -= 8;
    };             
  };
  if (isatty(1)) {
    INFO(("\r                           \r"));
  } else {
    INFO(("ok\n"));
  };          
  DEBUG(("peeked %d bytes",bytes_peeked));
}
#else
{
   BYTE Buf[5], *c;
   INT32 a;
   INT32 l;
   static BYTE Flicks[]="|\\-/|\\-/" ;
   int Flick=0;
   register int BytesRead,BytesWritten;

   INFO(( "Peeking root transputer..." ));
   Buf[0] = PEEK_TRANSPUTER;
   for( a = 0L ; a < CoreSize ; )
      {
         l = (a + 0x80000000L);
         c = (BYTE *)&l;
#ifdef BIG_ENDIAN
         Buf[4]= *c++ ; Buf[3]= *c++ ; Buf[2]= *c++ ; Buf[1]= *c;
#else
         Buf[1]= *c++ ; Buf[2]= *c++ ; Buf[3]= *c++ ; Buf[4]= *c;
#endif
         BytesWritten = WriteLink(TheLink, &Buf[0], 5, TIMEOUT);
         if (BytesWritten != 5)
         {
            if (BytesWritten < 0)
               ABORT(MISC_EXIT, (SE, "timed out peeking word %d because\n%s", a/4, LinkMessages[0-BytesWritten]))
            else
               ABORT(MISC_EXIT, (SE, "timed out peeking word %d", a/4 ));
         }

         BytesRead = ReadLink(TheLink, &Buf[1], 4, TIMEOUT);
         if (BytesRead != 4)
         {
            if (BytesRead < 0)
               ABORT(MISC_EXIT, (SE, "timed out in peek of word %d because\n%s", 
                                     a/4, LinkMessages[0-BytesRead]))
            else
               ABORT(MISC_EXIT, (SE, "timed out in peek of word %d", a/4));
         }

         CoreDump[a++] = Buf[1];
         CoreDump[a++] = Buf[2];
         CoreDump[a++] = Buf[3];
         CoreDump[a++] = Buf[4];

         if ( isatty(1) )
            if ( (a & 255L) == 0L )
               {
                  INFO(( "%c\b", Flicks[Flick] ));
                  if ( ++Flick == 8 ) Flick -= 8;
               }
      }
   if ( isatty(1) )
      {
         INFO(( "\r                           \r" ));
      }
   else
      {
         INFO(("ok\n"));
      }
   DEBUG(( "peeked %ld bytes", a ));
}
#endif




/*
 *   PrintHelp 
 */


PRIVATE VOID PrintHelp ()
{
   fprintf( stdout, "\n%s : occam 2 toolset host file server\n", PROGRAM_NAME);
   fprintf( stdout, "%s&%s version %s.\n", HOST, HardwareNames[BOARD_ID], VERSION_NAME );
   fprintf( stdout, "%s\n", Copyright );
   fprintf( stdout, "Usage:  %s {%coption}\n\n", PROGRAM_NAME, SWITCH_CHAR );
   fprintf( stdout, "Options: \n" );
   fprintf( stdout, "        SB filename   boot the named file (same as %cSR%cSS%cSI%cSC filename)\n", SWITCH_CHAR, SWITCH_CHAR, SWITCH_CHAR, SWITCH_CHAR );
   fprintf( stdout, "        SI            verbose mode\n");
   fprintf( stdout, "        SE            test the error flag\n");
   fprintf( stdout, "        SL linkname   use the named link\n");
   fprintf( stdout, "        SR            reset the root transputer\n");
   fprintf( stdout, "        SA            analyse and peek the root transputer\n");
   fprintf( stdout, "        SC filename   copy the named file to the link\n");
   fprintf( stdout, "        SP n          set peek size to n Kbytes.\n");
   fprintf( stdout, "        SS            serve the link\n\n");
}



/*
 *   ParseCommandLine 
 */

PRIVATE VOID ParseCommandLine ()
{
   register BYTE *s, *t;
   int i;
   BOOL Gobble=TRUE;

   AnalyseSwitch = 0;
   TestErrorSwitch = 0;
   VerboseSwitch = 1; 
   LinkSwitch = 0;
   ResetSwitch = 0;
   ServeSwitch = 0;
   LoadSwitch = 0;
   CocoPops = 0;
   CoreSize = DEFAULT_CORE_SIZE;

   BootFileName[0]=0;
   LinkName[0]=0;

   s = RealCommandLine;
   t = DoctoredCommandLine;
   
   while( (*s) && (*s != ' ') )                                            /* skip the first argv */
      ++s;
   if ( *s == ' ' )
      ++s;

   for(;;)
      {
         if ( *s == NULL )                                                 /*  end of command line  */
           return;
         if (  (*s==SWITCH_CHAR) && ( (*(s+1)=='s') || (*(s+1)=='S') )  )
            {         /*  its a server option  */
               s += 2;
               switch ( *s )
                  {
                     case 'a' :
                     case 'A' :  ++AnalyseSwitch; ++s; break;

                     case 'b' :
                     case 'B' :
                                 ++s; ++ResetSwitch; ++LoadSwitch; ++ServeSwitch; ++VerboseSwitch;
                                 while( *s == ' ' )
                                    ++s;
                                 i = 0;
                                 while( (*s) && (*s != SWITCH_CHAR) && (*s != ' ') )
                                    {
                                       if ( i == MAX_BOOT_FILE_LENGTH )
                                          ABORT(MISC_EXIT, (SE, "boot filename is too long, maximum size is %d characters", MAX_BOOT_FILE_LENGTH ));
                                       BootFileName[i++] = *s++;
                                    }
                                 if ( i == 0 )
                                    ABORT(MISC_EXIT, (SE, "expected a filename after %csb option", SWITCH_CHAR ));
                                 BootFileName[i] = 0;
                                 break;

                     case 'c' :
                     case 'C' :
                                 ++s; ++LoadSwitch;
                                 while( *s == ' ' )
                                    ++s;
                                 i = 0;
                                 while( (*s) && ( (*s!=SWITCH_CHAR) && (*s!=' ') ) )
                                    {
                                       if ( i == MAX_BOOT_FILE_LENGTH )
                                          ABORT(MISC_EXIT, (SE, "copy filename is too long, maximum size is %d characters", MAX_BOOT_FILE_LENGTH ));
                                       BootFileName[i++] = *s++;
                                    }
                                 if ( i == 0 )
                                    ABORT(MISC_EXIT, (SE, "expected a filename after %csc option", SWITCH_CHAR ));
                                 BootFileName[i] = 0;
                                 break;

                     case 'e' :
                     case 'E' :  ++s; ++TestErrorSwitch; break;

                     case 'i' :
                     case 'I' :  ++s; ++VerboseSwitch; break;

                     case 'l' :
                     case 'L' :  ++s; ++LinkSwitch;
                                 while( *s == ' ' )
                                    ++s;
                                 i = 0;
                                 while( (*s) && ( (*s!=SWITCH_CHAR) && (*s!=' ') ) )
                                    {
                                       if ( i == MAX_LINK_NAME_LENGTH )
                                          ABORT(MISC_EXIT, (SE, "link name is too long, maximum size is %d characters", MAX_LINK_NAME_LENGTH ));
                                       LinkName[i++] = *s++;
                                    }
                                 if ( i == 0 )
                                    ABORT(MISC_EXIT, (SE, "expected a name after %csl option", SWITCH_CHAR ));
                                 LinkName[i] = 0;
                                 break;
                                 
                     case 'p' :
                     case 'P' :  ++s;
                                 while (*s == ' ')
                                    ++s;
                                 CoreSize = (atoi(s) * 1024);
                                 if (CoreSize == 0)
                                    ABORT(MISC_EXIT, (SE, "expected a number after %cSP option\n", SWITCH_CHAR));
                                 while( (*s) && (*s!=SWITCH_CHAR) )
                                    ++s;
                                 break;

                     case 'r' :
                     case 'R' :  ++s; ++ResetSwitch; break;

                     case 's' :
                     case 'S' :  ++s; ++ServeSwitch; break;

                     case 'z' :
                     case 'Z' :  ++s; ++CocoPops; break;

                     default  :  
                                 *t++ = SWITCH_CHAR;
                                 *t++ = *(s-1);
                                 Gobble = FALSE;
                                 break;
                  }
               if ( (Gobble == TRUE) && (*s == ' ') )
                  ++s;
            }
         else
            {
               *t++ = *s++;
            }
      }
}




/*
 *   Serve  -  the main loop (read buffer from link, call a function, write buffer to link)
 */

PRIVATE int Serve ()
{
   register int Count, Size;
   BOOL TerminateFlag = FALSE;
   int Result;
   
   for(;;)
      {
         if( TestErrorSwitch )
            if( TestError( TheLink ) == 1 )
               ABORT(ERROR_FLAG_EXIT,(SE, "Transputer error flag set"));
         DEBUG(("-=-"));
         for(;;)
            {
               Count = ReadLink( TheLink, &Tbuf[0], 8, TIMEOUT );
               if( TestErrorSwitch )
                  if( TestError( TheLink ) == 1 )
                     ABORT(ERROR_FLAG_EXIT, (SE, "Error flag raised by transputer"));
               if( Count > 0 )
                  {
                     if ( Count == 8 )
                        break;
                     Count += ReadLink( TheLink, &Tbuf[Count], 8-Count, TIMEOUT );
                     if( Count != 8 )
                        ABORT(MISC_EXIT, (SE, "protocol error, got %d bytes at start of a transaction", Count ));
                  }
               else if (Count < 0)
                  ABORT(MISC_EXIT, (SE, "Unable to get request from link because\n%s", LinkMessages[0-Count]))
               else /* Count == 0 */
                  DEBUG(("ReadLink retrying"));

               DEBUG(("waiting"));
#ifdef MSC
               kbhit();                                     /*  test break  */
#endif
            }
                                                            /*  have 8  */
         Size = Tbuf[0]+Tbuf[1]*256-6 ;
         if ( Size )
            {
               if ( Size > (TRANSACTION_BUFFER_SIZE - 8 ))
                  ABORT(MISC_EXIT, (SE, "protocol error, packet size is too large"));
               if ( Size < 0 )
                  ABORT(MISC_EXIT, (SE, "protocol error, read nonsense from the link"));
               Count = ReadLink( TheLink, &Tbuf[8], Size, TIMEOUT );
               if( TestErrorSwitch )
                  if( TestError( TheLink ) == 1 )
                     ABORT(ERROR_FLAG_EXIT,(SE,"Error flag raised by transputer"));
               if( Count != Size )
               {
                  if (Count < 0)
                     ABORT(MISC_EXIT, (SE, "protocol error, timed out getting a further %d because\n%s", 
                                             Count, LinkMessages[0-Count]))
                  else
                     ABORT(MISC_EXIT, (SE, "protocol error, timed out getting a further %d", Count));
               }
               DEBUG(( "packet is %d", Count+6 ));
            }

         switch( Tbuf[2] )
            {
               case SP_OPEN     : SpOpen();     break; 
               case SP_CLOSE    : SpClose();    break; 
               case SP_READ     : SpRead();     break; 
               case SP_WRITE    : SpWrite();    break; 
               case SP_GETBLOCK : SpGetBlock(); break;
               case SP_PUTBLOCK : SpPutBlock(); break;
               case SP_GETS     : SpGets();     break; 
               case SP_PUTS     : SpPuts();     break; 
               case SP_FLUSH    : SpFlush();    break; 
               case SP_SEEK     : SpSeek();     break; 
               case SP_TELL     : SpTell();     break; 
               case SP_EOF      : SpEof();      break; 
               case SP_FERROR   : SpError();    break; 
               case SP_REMOVE   : SpRemove();   break; 
               case SP_RENAME   : SpRename();   break; 
               case SP_GETKEY   : SpGetkey();   break; 
               case SP_POLLKEY  : SpPollkey();  break; 
               case SP_GETENV   : SpGetenv();   break; 
               case SP_TIME     : SpTime();     break; 
               case SP_SYSTEM   : SpSystem();   break; 
               case SP_EXIT     : TerminateFlag = TRUE;
                                  Result = SpExit();
                                  TestErrorSwitch = FALSE;
                                  break;
               case SP_COMMAND  : SpCommand();  break; 
               case SP_CORE     : SpCore();     break; 
               case SP_ID       : SpId();       break; 
#ifdef MSC
               case SP_MSDOS    : SpMsdos();    break;
#endif
               default          :
                  SpUnknown(); break;
            }

         Size = Tbuf[0]+Tbuf[1]*256+2 ;
         if ( Size )
            {
               if ( Size > TRANSACTION_BUFFER_SIZE )
                  ABORT(MISC_EXIT, (SE, "internal error, oversize packet write back"));
               DEBUG(( "%d:%d", Size-2, Tbuf[2] ));
               Count = WriteLink( TheLink, &Tbuf[0], Size, TIMEOUT );
               if( TestErrorSwitch )
                  if( TestError( TheLink ) == 1 )
                     ABORT(ERROR_FLAG_EXIT,(SE,"Error flag raised by transputer"));
               if( Count != Size )
               {
                  if (Count < 0)
                     ABORT(MISC_EXIT, (SE, "protocol error, timed out sending reply message because\n%s",
                                            LinkMessages[0-Count]))
                  else
                     ABORT(MISC_EXIT, (SE, "protocol error, timed out sending reply message"));
               }
            }
         else
            ABORT(MISC_EXIT, (SE, "internal error, zero size write back"));
            
         if( TerminateFlag == TRUE )
            return( Result );
      }
}




PUBLIC void BreakHandler()
{
   BYTE c = 0;

#ifndef SUN
   signal(SIGINT, BreakHandler);
#endif

   fputc('\n', stderr); 
   while ( c == 0 )
      {
         fprintf(stderr,"%s: (x)exit, (s)hell or (c)ontinue? ", PROGRAM_NAME); fflush(stderr);
         c = GetAKey(); 
         fputc(c, stderr);
         if ( (c != 10) && (c != 13) )
            fputc('\n', stderr); 
         fflush(stderr);
         switch(c)
            {
               case  0 :
               case 10 :
               case 13 :
               case 'x':
               case 'X':
               case 'q':
               case 'Q':
                          DEBUG(("break exit"));
                          if (CloseLink(TheLink) < 0)
                             ABORT(MISC_EXIT, (SE, "unable to free transputer link"));
                          ABORT(USER_EXIT, (SE, "aborted by user"));
               case 'c':
               case 'C':
                          break;
               case 'S':
               case 's':
                          ResetTerminal();
#ifdef MSC
                          (void)system(getenv("COMSPEC"));
#endif
#ifdef SUN
                          (void)system(getenv("SHELL"));
#endif
#ifdef HELIOS
                          (void)system(getenv("SHELL"));
#endif
#ifdef VMS
                          {
                             long substatus;
                             INFO(("[Starting DCL]"));
                             (void)lib$spawn(NULL, 0,0,0,0,0,&substatus,0,0,0);
                          }
#endif
                          fprintf(stderr,"\n");
                          break;

               case 'i':
                          if (VerboseSwitch)
                             VerboseSwitch = 0;
                          else
                             VerboseSwitch = 1;
                          break;

               case 'z':
                          if (CocoPops)
                             CocoPops = 0;
                          else
                             CocoPops = 1;
                          break;
   
               default:
                          c = 0;
                          break;
            }
      }
}


PRIVATE int QuotedArg(str)
char *str;
{
   while (*str) {
      if ((*str == ' ') || (*str == '\t'))
         return TRUE;

      str++;
   }

   return FALSE;
}


/*
 *   main 
 */


PUBLIC int main (argc, argv)
   int argc;
   char **argv;
{
   int Quoted;
   int ExitStatus = MISC_EXIT;
   int LinkStatus;
   BYTE *c, *ALinkName;

   if ( argc == 1 )
      {
         PrintHelp();
         return ( MISC_EXIT );
      }

   HostBegin();                        /*  terminal initialisation  */

   *RealCommandLine = 0;
   while ( argc-- > 0 )
      {
         if ( ( MAX_COMMAND_LINE_LENGTH - strlen( RealCommandLine) ) < strlen( *argv ) )
            {
               ABORT(MISC_EXIT, (SE, "Command line too long (at \"%s\")", *argv) )
            }
         if (QuotedArg(*argv)) {
            Quoted = TRUE;
            (void)strcat( RealCommandLine, "\"" );
         }
         else
            Quoted = FALSE;

         (void)strcat( RealCommandLine, *argv );
         if (Quoted)
            (void)strcat( RealCommandLine, "\"");
         (void)strcat( RealCommandLine, " " );
         ++argv;
      }
   for ( c=RealCommandLine; *c ; ++c )                                     /*  strip of the last space  */
      ;
   if ( *--c == ' ' )
      *c = 0;

   ParseCommandLine();
   DEBUG(("original line is \"%s\"", RealCommandLine));
   DEBUG(("doctored line is \"%s\"", DoctoredCommandLine));

   signal( SIGINT, BreakHandler );
#ifdef SUN
   signal( SIGTSTP, SIG_IGN );                                             /*  disable ctrl-z  */
#endif

   if ( CocoPops )
      printf("(analyse=%d error=%d verbose=%d link=%d reset=%d serve=%d load=%d coco=%d)\n", AnalyseSwitch, TestErrorSwitch, VerboseSwitch, LinkSwitch, ResetSwitch, ServeSwitch, LoadSwitch, CocoPops );

   DEBUG(("peek size = %ld bytes\n", CoreSize));
   if ((CoreDump = (BYTE *) malloc(CoreSize)) == NULL)
      ABORT(MISC_EXIT, (SE, "failed to allocate CoreDump buffer\n"));

   DEBUG(("using \"%s\" as the boot file", BootFileName));
      
   if ( *LinkName == 0 )
      {
         if ( ( ALinkName = (BYTE *)getenv("TRANSPUTER") ) != NULL )
            strcpy( LinkName, ALinkName );
      }
      
   DEBUG(("and \"%s\" as the link name", LinkName));

   if ( ( TheLink = OpenLink( LinkName ) ) < 1 )
      if(TheLink == ER_LINK_BAD){
         ABORT(MISC_EXIT, (SE, "Bad link specification"));
      }
      else{
         ABORT(MISC_EXIT, (SE, "unable to access a transputer"));
      }

   if ( ResetSwitch && AnalyseSwitch )
      ABORT(MISC_EXIT, (SE, "reset and analyse are incompatible"));

   if ( ResetSwitch )
      {
         LinkStatus = ResetLink(TheLink);
         if (LinkStatus != 1)
            ABORT(MISC_EXIT, (SE, "failed to reset root transputer because\n%s", 
                                                      LinkMessages[0-LinkStatus]));
         DEBUG(("reset root"));
      }

   if ( AnalyseSwitch )
      {
         LinkStatus = AnalyseLink(TheLink);
         if (LinkStatus != 1)
            ABORT(MISC_EXIT, (SE, "failed to analyse root transputer because\n%s",
                                                      LinkMessages[0-LinkStatus]));                                                    
         DEBUG(("analyse root"));
         Core();
      }

   if ( LoadSwitch )
      Boot();

   if ( ServeSwitch )
      ExitStatus = Serve();
   else   
      {
         if ( ResetSwitch || AnalyseSwitch || LoadSwitch || TestErrorSwitch )
            ExitStatus = MISC_EXIT;
         else
            {
               INFO(("(no action taken)\n"));
            }
      }

   if( TestErrorSwitch )
      if( TestError( TheLink ) == 1 )
         ABORT(ERROR_FLAG_EXIT,(SE,"Error flag raised by transputer"));

   signal(SIGINT, SIG_IGN);                    /*  dont want to be interrupted while closing  */
   
   LinkStatus = CloseLink(TheLink);
   if (LinkStatus < 1)
      ABORT(MISC_EXIT, (SE, "unable to free transputer link because\n%s", 
                                                LinkMessages[0-LinkStatus]));                                             

   HostEnd();

   return( ExitStatus );
}



/*
 *   Eof
 */

