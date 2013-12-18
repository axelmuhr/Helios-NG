
/*
 --   ---------------------------------------------------------------------------
 --
 --      ISERVER  -  INMOS standard file server
 --
 --      filec.c
 --
 --      Primary file operations
 --
 --      Copyright (c) INMOS Ltd., 1988.
 --      All Rights Reserved.
 --
 --   ---------------------------------------------------------------------------
*/



#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef VMS
#include <unixio.h>
#endif

#include "inmos.h"
#include "iserver.h"
#include "pack.h"

 
EXTERN BOOL CocoPops;							   /*  for DEBUG  */
EXTERN BOOL VerboseSwitch;

EXTERN int errno;

EXTERN BYTE Tbuf[TRANSACTION_BUFFER_SIZE];

PRIVATE BYTE DataBuffer[MAX_SLICE_LENGTH+1];
PRIVATE int Size;




/*
 *   SpOpen
 */

PUBLIC VOID SpOpen()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   BYTE *Name, Type, Mode;

   DEBUG(( "SP.OPEN" ));
   INIT_BUFFERS;

   Name = &DataBuffer[0];
   GET_SLICE( Size, Name ); *(Name+Size)=0; DEBUG(( "\"%s\"", Name ));
   
   GET_BYTE( Type ); DEBUG(( "type %d", Type ));
   GET_BYTE( Mode ); DEBUG(( "mode %d", Mode ));

   if ( strlen( Name ) == 0 )
      {
         PUT_BYTE( SP_ERROR );
         REPLY;
      }
   
   if ( ( Type != 1 ) && ( Type !=2 ) )
      {
         PUT_BYTE( SP_ERROR );
         REPLY;
      }

   if( ( Mode < 0 ) || ( Mode > 6 ))
      {
         PUT_BYTE( SP_ERROR );
         REPLY;
      }

#ifdef SUN								   /*  fopen file modes  */
#define BINARY_1 "r"
#define BINARY_2 "w"
#define BINARY_3 "a"
#define BINARY_4 "r+"
#define BINARY_5 "w+"
#define BINARY_6 "a+"
#define TEXT_1 BINARY_1 
#define TEXT_2 BINARY_2 
#define TEXT_3 BINARY_3 
#define TEXT_4 BINARY_4 
#define TEXT_5 BINARY_5 
#define TEXT_6 BINARY_6 
#endif

#ifdef MSC
#define BINARY_1 "rb"
#define BINARY_2 "wb"
#define BINARY_3 "ab"
#define BINARY_4 "r+b"
#define BINARY_5 "w+b"
#define BINARY_6 "a+b"
#define TEXT_1 "rt"
#define TEXT_2 "wt"
#define TEXT_3 "at"
#define TEXT_4 "r+t"
#define TEXT_5 "w+t"
#define TEXT_6 "a+t"
#endif

#ifdef HELIOS
#define BINARY_1 "rb"
#define BINARY_2 "wb"
#define BINARY_3 "ab"
#define BINARY_4 "r+b"
#define BINARY_5 "w+b"
#define BINARY_6 "a+b"
#define TEXT_1 "r"
#define TEXT_2 "w"
#define TEXT_3 "a"
#define TEXT_4 "r+"
#define TEXT_5 "w+"
#define TEXT_6 "a+"
#endif

#ifdef VMS
#define BINARY_1 "rb"
#define BINARY_2 "wb"
#define BINARY_3 "ab"
#define BINARY_4 "r+b"
#define BINARY_5 "w+b"
#define BINARY_6 "a+b"
#define TEXT_1 "rt"
#define TEXT_2 "wt"
#define TEXT_3 "at"
#define TEXT_4 "r+t"
#define TEXT_5 "w+t"
#define TEXT_6 "a+t"
#endif

   if( Type == 1 )
      switch( Mode )
         {
            case 1 : Fd = fopen( Name, BINARY_1 ); break;
            case 2 : Fd = fopen( Name, BINARY_2 ); break;
            case 3 : Fd = fopen( Name, BINARY_3 ); break;
            case 4 : Fd = fopen( Name, BINARY_4 ); break;
            case 5 : Fd = fopen( Name, BINARY_5 ); break;
            case 6 : Fd = fopen( Name, BINARY_6 ); break;
         }
   else
      switch( Mode )
         {
            case 1 : Fd = fopen( Name, TEXT_1 ); break;
            case 2 : Fd = fopen( Name, TEXT_2 ); break;
            case 3 : Fd = fopen( Name, TEXT_3 ); break;
            case 4 : Fd = fopen( Name, TEXT_4 ); break;
            case 5 : Fd = fopen( Name, TEXT_5 ); break;
            case 6 : Fd = fopen( Name, TEXT_6 ); break;
         }

   if( Fd == NULL )
      {
         PUT_BYTE( SP_ERROR );
         REPLY;
      }

   PUT_BYTE( SP_SUCCESS );
   PUT_FD( Fd )
   DEBUG(( "fd %0X", (int)Fd ));
   REPLY;
}




/*
 *   SpClose
 */

PUBLIC VOID SpClose()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;

   DEBUG(( "SP.CLOSE" ));
   INIT_BUFFERS;

   GET_FD( Fd ); DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin; break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }

   if ( fclose( Fd ) == EOF )
      {
         PUT_BYTE( SP_ERROR );
      }
   else
      {
         PUT_BYTE( SP_SUCCESS );
      }
   REPLY;
}




/*
 *   SpRead
 */

PUBLIC VOID SpRead()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   BYTE *Data;
   int Read;

   DEBUG(( "SP.READ" ));
   INIT_BUFFERS;

   GET_FD( Fd ); DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin;
#ifdef SUN
                  ResetTerminal();
#endif
                  break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }

   GET_INT16( Size ); DEBUG(( "request %d", Size ));

   Data = &DataBuffer[0];
   Read = fread( Data, 1, Size, Fd );
   DEBUG(( "read %d", Read ));
   PUT_BYTE( SP_SUCCESS );
   PUT_SLICE( Read, Data );

   REPLY;   
}



/*
 *   SpWrite
 */

PUBLIC VOID SpWrite()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   BYTE *Data;
   int Written;
         
   DEBUG(( "SP.WRITE" ));
   INIT_BUFFERS;

   GET_FD( Fd ); DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin; break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }

   Data = &DataBuffer[0];
   GET_SLICE( Size, Data ); DEBUG(( "%d bytes", Size ));
   
#ifdef VMS
                                 /*  VMS RMS generates a record for *each* item  */
   if ( fwrite( Data, Size, 1, Fd ) != 1 )
      Written = 0;
   else
      Written = Size;
#else
   Written = fwrite( Data, 1, Size, Fd );
#endif
   if ( Fd == stdout )
      fflush( stdout );
   DEBUG(( "wrote %d", Written ));
   PUT_BYTE( SP_SUCCESS );
   PUT_INT16( Written );
   REPLY;      
}




/*
 *   SpGetBlock
 */

PUBLIC VOID SpGetBlock()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   BYTE *Data;
   int Read;

   DEBUG(( "SP.GETBLOCK" ));
   INIT_BUFFERS;

   GET_FD( Fd ); DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin;
#ifdef SUN
                  ResetTerminal();
#endif
                  break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }

   GET_INT16( Size ); DEBUG(( "request %d", Size ));

   Data = &DataBuffer[0];
   Read = fread( Data, 1, Size, Fd );
   DEBUG(( "read %d", Read ));
   PUT_BYTE( (Read == 0) ? SP_ERROR : SP_SUCCESS );
   PUT_SLICE( Read, Data );

   REPLY;   
}



/*
 *   SpPutBlock
 */

PUBLIC VOID SpPutBlock()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   BYTE *Data;
   int Written;
         
   DEBUG(( "SP.PUTBLOCK" ));
   INIT_BUFFERS;

   GET_FD( Fd ); DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin; break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }

   Data = &DataBuffer[0];
   GET_SLICE( Size, Data ); DEBUG(( "%d bytes", Size ));
   
#ifdef VMS
                                 /*  VMS RMS generates a record for *each* item  */
   if ( fwrite( Data, Size, 1, Fd ) != 1 )
      Written = 0;
   else
      Written = Size;
#else
   Written = fwrite( Data, 1, Size, Fd );
#endif
   if ( Fd == stdout )
      fflush( stdout );
   DEBUG(( "wrote %d", Written ));
   PUT_BYTE( (Written == 0) ? SP_ERROR : SP_SUCCESS );
   PUT_INT16( Written );
   REPLY;      
}




/*
 *   SpGets
 */

PUBLIC VOID SpGets()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   BYTE *Data;

   DEBUG(( "SP.GETS" ));
   INIT_BUFFERS;

   GET_FD( Fd ); DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin;
#ifdef SUN
                  ResetTerminal();
#endif
                  break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }
   GET_INT16( Size ); DEBUG(( "limit %d", Size ));

   Data = &DataBuffer[0];
   if ( fgets( Data, Size, Fd ) == NULL )
      {
         PUT_BYTE( SP_ERROR );
      }
   else
      {
         Size = strlen( Data );
         if( *(Data+Size-1) == '\n')
            {
               *(Data+Size) = 0;
               --Size;
            }
         DEBUG(( "got %d", Size ));
         PUT_BYTE( SP_SUCCESS );
         PUT_SLICE( Size, Data );
      }
   REPLY;
}




/*
 *   SpPuts
 */

PUBLIC VOID SpPuts()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   BYTE *Data;

   DEBUG(( "SP.PUTS" ));
   INIT_BUFFERS;

   GET_FD( Fd ); DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin; break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }

   Data = &DataBuffer[0];
   GET_SLICE( Size, Data ); DEBUG(( "%d bytes", Size ));

   *(Data+Size)=0;
   if ( fputs( Data, Fd ) == EOF )
      {
         PUT_BYTE( SP_ERROR );
      }
   else
      {
         if( fputs( "\n", Fd ) == EOF )
            {
               PUT_BYTE( SP_ERROR );
            }
         else
            {
               PUT_BYTE( SP_SUCCESS );
            }
      }
   REPLY;
}




/*
 *   SpFlush
 */

PUBLIC VOID SpFlush()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   
   DEBUG(( "SP.FLUSH" ));
   INIT_BUFFERS;

   GET_FD( Fd ); DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin; break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }
   if( fflush( Fd ) == EOF )
      {
         PUT_BYTE( SP_ERROR );
      }
   else
      {
         PUT_BYTE( SP_SUCCESS );
      }
   REPLY;
}




/*
 *   SpSeek
 */

PUBLIC VOID SpSeek()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   INT32 Offset, Origin;
   int origin;

   DEBUG(( "SP.SEEK" ));
   INIT_BUFFERS;

   GET_FD( Fd );
   DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin; break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }
   GET_INT32( Offset ); DEBUG(( "offset %ld", Offset ));
   GET_INT32( Origin ); DEBUG(( "origin %ld", Origin ));

#ifdef SUN
   origin = (int)--Origin;
#endif
#ifdef VMS
   origin = (int)--Origin;
#endif
#ifdef MSC
   switch( (int)Origin )
      {
         case 1 : origin = SEEK_SET; break;
         case 2 : origin = SEEK_CUR; break;
         case 3 : origin = SEEK_END; break;
      }
#endif
   if ( fseek( Fd, Offset, origin ) )
      {
         PUT_BYTE( SP_ERROR );
      }
   else
      {
         PUT_BYTE( SP_SUCCESS );
      }
   REPLY;
   
}




/*
 *   SpTell
 */

PUBLIC VOID SpTell()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   long Position;

   DEBUG(( "SP.TELL" ));
   INIT_BUFFERS;

   GET_FD( Fd );
   DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin; break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }

   Position = ftell( Fd );
   if( Position == -1L )
      {
         PUT_BYTE( SP_ERROR );
      }
   else
      {
         PUT_BYTE( SP_SUCCESS );
         PUT_INT32( Position );
      }
   REPLY;
}




/*
 *   SpEof
 */

PUBLIC VOID SpEof()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;

   DEBUG(( "SP.EOF" ));
   INIT_BUFFERS;

   GET_FD( Fd );
   DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin; break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }
   if( feof( Fd ) )
      {
         PUT_BYTE( SP_SUCCESS );
      }
   else
      {
         PUT_BYTE( SP_ERROR );
      }
   REPLY;
}




/*
 *   SpError
 */

PUBLIC VOID SpError()
{
   BUFFER_DECLARATIONS;
   FILE *Fd;
   INT32 Errno;
   BYTE String[128];

   DEBUG(( "SP.ERROR" ));
   INIT_BUFFERS;

   GET_FD( Fd );
   DEBUG(( "fd %0X", (int)Fd ));
   switch( (int)Fd )
      {
         case 0 : Fd = stdin; break;
         case 1 : Fd = stdout; break;
         case 2 : Fd = stderr; break;
      }

   if( ferror(Fd) )
      {
         PUT_BYTE( SP_SUCCESS );
         Errno = errno;
         DEBUG(( "errno %d", Errno ));
         PUT_INT32( Errno );
#ifdef SUN
	 String[0] = 0;
#else
         strcpy( &String[0], strerror(errno));
#endif
         DEBUG(( "error \"%s\"", &String[0]));
         Size = strlen(&String[0]);
         PUT_SLICE( Size, String[0] );
         REPLY;
      }
   else
      {
         PUT_BYTE( SP_ERROR );
         REPLY;
      }
}




/*
 *   SpRemove
 */

PUBLIC VOID SpRemove()
{
   BUFFER_DECLARATIONS;
   BYTE *Name;

   DEBUG(( "SP.REMOVE" ));
   INIT_BUFFERS;

   Name = &DataBuffer[0];
   GET_SLICE( Size, Name ); DataBuffer[Size] = 0; DEBUG(( "\"%s\"", Name ));
   if ( *Name == 0 )
      {
         PUT_BYTE( SP_ERROR );
      }
   else
      {
#ifdef SUN
         if( unlink( Name ) )
#else
         if( remove( Name ) )
#endif
            {
               PUT_BYTE( SP_ERROR );
            }
         else
            {
               PUT_BYTE( SP_SUCCESS );
            }
      }
   REPLY;
}



/*
 *   SpRename
 */

PUBLIC VOID SpRename()
{
   BUFFER_DECLARATIONS;
   BYTE *Oldname, *Newname;

   DEBUG(( "SP.RENAME" ));
   INIT_BUFFERS;
   Oldname = &DataBuffer[0];
   GET_SLICE( Size, Oldname ); *(Oldname+Size)=0; DEBUG(( "old \"%s\"", Oldname ));

   Newname = Oldname+Size+1;
   GET_SLICE( Size, Newname ); *(Newname+Size)=0; DEBUG(( "new \"%s\"", Newname ));

   if ( *Oldname == 0 )
      {
         PUT_BYTE( SP_ERROR );
      }
   else
      {
         if ( *Newname == 0 )
            {
               PUT_BYTE( SP_ERROR );
            }
          else
            {
               if( rename( Oldname, Newname ) )
                  {
                     PUT_BYTE( SP_ERROR );
                  }
               else
                  {
                     PUT_BYTE( SP_SUCCESS );
                  }
            }
      }
   REPLY;

}



/*
 *   Eof
 */

