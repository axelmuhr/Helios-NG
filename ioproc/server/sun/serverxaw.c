/*
 * serverXaw.c : Athena Widgets based front end for the
 *               Helios I/O Server
 *
 *   Copyright (c) 1993 Perihelion Software Ltd.
 *     All rights reserved.
 *
 * Author :	N Clifton
 * Version :	$Revision: 1.4 $
 * Date :	$Date: 1994/01/14 10:20:37 $
 * Id :		$Id: serverxaw.c,v 1.4 1994/01/14 10:20:37 nickc Exp $
 */

/*{{{  Description     */

/*
 * Hello and welcome to the Athena Widget Front End to the UNIX based
 * Helios I/O Server.  Due to limitiations in the Athena Widget
 * Library (basically it does not have a terminal widget emulator
 * widget, and I had no wish to create one), the xterm program is used
 * to perform terminal emulation.  The scheme works as follows:
 *
 * 1) The IO server does a vfork() and exec() of 'serverXaw'.
 *    ServerXaw inherits an open file descriptor which is its
 *    connection to the IO server.  The file descriptor is identified
 *    by a command line argument.
 *
 * 2) ServerXaw creates a socket and binds it to a port.
 *
 * 3) ServerXaw does a vfork() and exec() of 'xterm'.
 *    Xterm closes all its open file descriptors, creates a window etc.
 *
 * 4) Xterm does a vfork() and exec() of 'serverXaw'.
 *    This second copy of serverXaw is passed a command line argument
 *    identifying the port number of the socket created by the first
 *    serverXaw.
 *
 * 5) The second serverXaw connects to the port created by the first
 *    serverXaw, and then channels input from the socket to the xterm
 *    process (which has attached itself to the stdin and stdout of
 *    the second serverXaw process).
 *
 * Points to note:
 *
 * The IO server cannot exec() xterm directly because xterm closes all
 * of its open file descriptors first, before forking off its child
 * process.
 *
 * The first serverXaw process cannot talk to the xterm process,
 * because Xterm only talks to the child process that it creates.
 *
 * The whole thing looks like this:
 *
 *  ----------- 	   -----------           -----------           -------
 * | IO Server | <------> | serverXaw | <-----> | serverXaw | <-----> | xterm |
 *  -----------     ^      -----------     ^     -----------     ^     -------
 *     	       	    |	       	       	   |	   		 |
 *		    |	    		   |	   		 |
 *            two way socket	     two way socket	   stdin & stdout
 *
 */

/*}}}*/
/*{{{  To Do           */

/*
 * Make Console Windows prompt users before exiting so that error messages can be read
 *
 * Find some way of detecting window resizes and sending them on to the IO server.
 */

/*}}}*/
/*{{{  Header files    */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifdef __STDC__

#include <stdarg.h>

#else /* not __STDC__ */

#include <varargs.h>

#endif /* __STDC__ */

#include <sys/types.h>
#ifndef SUN4
#include <sys/ioctl.h>	/* clashes with termios.h on SUNs */
#endif
#include <termio.h>	/* ioctl definitions */
#include <termios.h>	/* ioctl definitions */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Cardinals.h>

#include "sunlocal.h"

/* XXX - there does not appear to be a header file for termcap functions ! */

#ifdef __STDC__
extern int	tgetent( char aBuffer[ 1024 ], char * pTerminalName );
extern char *	tgetstr( char * pAttributeName, char ** ppBufferReturn );
extern char *	tgoto(   char * pcmString, int iColumn, int iRow );
#else
extern int	tgetent();
extern char *	tgetstr();
extern char *	tgoto();
#endif

/*}}}*/
/*{{{  Bitmaps         */

#include "ioserv.bm"

/*}}}*/
/*{{{  Constants       */

#if 0
#define DEBUG		0
#endif

#define NUM_ARGS	15		/* maximum number of arguments that can be passed to a toolkit fn */
#define IN_BUF_SIZE	4096		/* size of read buffer */

#ifndef PRIVATE
#define PRIVATE 	static
#define PUBLIC
#endif

#define PORT 		4567		/* magic port address - change if necessary */
#define MAXPORT		5000		/* maximum magic port number */
#define NUM_RETRIES	5		/* number of times to try to connect to a socket */

#ifdef RS6000
#define vfork fork
#endif

/*}}}*/
/*{{{  Macros          */

#define resetArgs_()		nXtArgs = 0

#define setArg_( type, value )	XtSetArg( pXtArgs[ nXtArgs ], type, value ); nXtArgs++

#define ctrl_( UpperCaseLetter ) (UpperCaseLetter - '@')

#ifdef DEBUG
#define DBG( text )		debug( text )
#else
#define DBG( text )
#endif

/*}}}*/
/*{{{  Local variables */

PRIVATE char *		ProgName;		/* obtained from arv[0]		*/
PRIVATE char *		TitleName = "Helios";
PRIVATE Boolean		show_debug     = FALSE;	/* enable debug menu iff TRUE	*/
PRIVATE Boolean		console_window = FALSE;	/* TRUE iff this is the I/O server's console window */
PRIVATE int		siphon         = 0;	/* port number of connection to better half */
PRIVATE int		to_io_server   = 0;	/* file descriptor for writes to I/O server */
PRIVATE int		from_io_server = 0;	/* file descriptor for reads from I/O server */
PRIVATE int		to_xterm       = 1;	/* file descriptor for writes to Xterm */
PRIVATE int		from_xterm     = 0;	/* file descriptor for reads from Xterm */

PRIVATE void		PanelInput		/* Forward declaration */
#ifdef __STDC__
  ( unsigned char panelEvent, unsigned char panelValue );
#else
  ( );
#endif

/*}}}*/
/*{{{  Functions       */

/*{{{  debug               */

/*
 * FUNCTION: debug
 *
 * ARGUMENTS:
 *	const char *	format	- printf(3) style format string
 *	...			- arguments for the message
 *
 * DESCRIPTION:
 * 	Display a formatted debugging message on stderr.
 * 	Automatically appends a new-line to the message.
 *	Automatically prepends the program's name (if it exists)
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE FILE *	pDebugFd = stderr;


PRIVATE void
debug
#ifdef __STDC__
  ( const char * format, ... )
#else
/*VARARGS*/
   ( format, va_alist )
  char *	format;
  va_dcl
#endif
{
  va_list	args;
	

  if (pDebugFd == NULL)
    {
      pDebugFd = fopen( "errs", "a+" );

      fprintf( pDebugFd, "******\n" );
    }
  
#ifdef __STDC__
  va_start( args, format );
#else
  va_start( args );
#endif

  if (ProgName)
    fprintf( pDebugFd, "%s: ", ProgName );
	
  vfprintf( pDebugFd, (char *)format, args );

  fputc( '\n', pDebugFd );
	
  va_end( args );

  return;
  
} /* debug */

/*}}}*/
/*{{{  warn                */

/*
 * FUNCTION: warn
 *
 * ARGUMENTS:
 *	const char *	format	- printf(3) style format string
 *	...			- arguments for the message
 *
 * DESCRIPTION:
 * 	Display a formatted warning message on stderr.
 * 	Automatically appends a new-line to the message.
 *	Automatically prepends the program's name (if it exists)
 *	Automatically prepends the string "Warning: "
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE void
warn
#ifdef __STDC__
  ( const char * format, ... )
#else
/*VARARGS*/
   ( format, va_alist )
  char *	format;
  va_dcl
#endif
{
  va_list	args;
	

  if (pDebugFd == NULL)
    {
      pDebugFd = fopen( "errs", "w" );      
    }
  
#ifdef __STDC__
  va_start( args, format );
#else
  va_start( args );
#endif

  if (ProgName)
    fprintf( pDebugFd, "%s: ", ProgName );

  fprintf( pDebugFd, "Warning: " );
  
  vfprintf( pDebugFd, (char *)format, args );

  fputc( '\n', pDebugFd );
	
  va_end( args );

  return;
  
} /* warn */

/*}}}*/
/*{{{  fail                */

/*
 * FUNCTION: fail
 *
 * ARGUMENTS:
 *	const char *	format	- printf(3) style format string
 *	...			- arguments for the message
 *
 * DESCRIPTION:
 * 	Display a formatted error message on stderr and
 *      then exits with an error condition.
 * 	Automatically appends a new-line to the message.
 *	Automatically prepends the program's name (if it exists)
 *	Automatically prepends the string "Failure: "
 *
 * RETURNS:
 * 	Does not return.
 */

PRIVATE void
fail
#ifdef __STDC__
  ( const char * format, ... )
#else
/*VARARGS*/
   ( format, va_alist )
char *	format;
va_dcl
#endif
{
  va_list	args;
	

  if (pDebugFd == NULL)
    {
      pDebugFd = fopen( "errs", "w" );      
    }
  
#ifdef __STDC__
  va_start( args, format );
#else
  va_start( args );
#endif

  if (ProgName)
    fprintf( pDebugFd, "%s: ", ProgName );

  fprintf( pDebugFd, "Failure: " );
  
  vfprintf( pDebugFd, (char *)format, args );

  fputc( '\n', pDebugFd );
	
  va_end( args );

  fclose( pDebugFd );
  
  exit( EXIT_FAILURE );

  return;
  
} /* fail */

/*}}}*/
/*{{{  usage               */

/*
 * FUNCTION: usage
 *
 * ARGUMENTS:
 *	None
 *
 * DESCRIPTION:
 *	Sends a description of the command line arguments
 *	understood by the program to stderr.
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE void
usage
#ifdef __STDC__
  ( void )
#else
  ()
#endif
{
  fprintf( stderr, "%s understands the following command line arguments ...\n",
	  ProgName == NULL ? "The program" : ProgName );

  fprintf( stderr, "-h                 - displays this information\n" );
  fprintf( stderr, "-D                 - toggle the display of the debug menu\n" );
  fprintf( stderr, "-c                 - this is an I/O server console window\n" );
  fprintf( stderr, "-p <n>             - file descriptor of pipe connection to/from I/O server\n" );
  fprintf( stderr, "-t <n>             - file descriptor of connection to I/O server\n" );
  fprintf( stderr, "-f <n>             - file descriptor of connection from I/O server\n" );
  
  fprintf( stderr, "\nplus all the X toolkit options, including ...\n" );
  fprintf( stderr, "-display <name>    - name of the X server to contact\n" );
  fprintf( stderr, "-geom <geometry>   - size and position of the window\n" );

  return;
  
} /* usage */

/*}}}*/
/*{{{  SafeWrite           */

/*
 * FUNCTION: SafeWrite
 *
 * ARGUMENTS:
 *	int	iFd	- connection number on which to perform the write
 *	char *	pBuffer	- buffer containing data to be written
 *	int	iLength - length of data in buffer
 *
 * DESCRIPTION:
 *	Writes the data in the buffer to the connection.
 *	Handles partial writes.
 *
 * RETURNS:
 * 	TRUE upon success, FALSE otherwise
 */

PRIVATE Bool
SafeWrite
#ifdef __STDC__
  ( int iFd, char * pBuffer, int iLength )
#else
  ( iFd, pBuffer, iLength )
int     iFd;
char *	pBuffer;
int	iLength;
#endif
{
  while (iLength > 0)
    {
      int	iSent;


      iSent = write( iFd, pBuffer, iLength );

      if (iSent < 0)
	{
	  warn( "Write failed, errno is %d", errno );

	  return FALSE;
	}

      iLength -= iSent;
      pBuffer += iSent;
    }

  return TRUE;
  
} /* SafeWrite */

/*}}}*/
/*{{{  PreProcessArguments */

/*
 * FUNCTION: PreProcessArguments
 *
 * ARGUMENTS:
 *	int	nArgs		- number of arguments passed in
 *	char **	ppArgs		- vector of argument strings
 *
 * DESCRIPTION:
 *	Processes special command line arguments created by
 *	the I/O server, but which do not conform to the
 *	normal spec for such arguments.
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE void
PreProcessArguments
#ifdef __STDC__
  ( int nArgs, char ** ppArgs )
#else
  ( nArgs, ppArgs )
int	   nArgs;
char **	   ppArgs;
#endif /* __STDC */
{
  int	arg;

  
  /* The first argument is our name */

  if (nArgs > 2 && isalpha( ppArgs[ 1 ][ 0 ] ))
    {
      ppArgs[ 0 ] = TitleName = ppArgs[ 1 ];
      ppArgs[ 1 ] = "";
    }

  /* if the second argument is alphabetic then it is because the first argument had a space in its name */
  
  if (nArgs > 3 && isalpha( ppArgs[ 2 ][ 0 ] ))
    {
      char * name;

      
      name = (char *)malloc( strlen( TitleName ) + 2 + strlen( ppArgs[ 2 ] ) );

      if (name == NULL)
	fail( "out of memory allocating program name" );

      sprintf( name, "%s %s", TitleName, ppArgs[ 2 ] );

      TitleName = name;	 /* XXX memory leak */

      arg = 3;

      ppArgs[ 2 ] = "";
    }
  else
    {
      arg = 2;
    }
  
  /* The second argument, is a numeric boolean indicating that we are a console window */
  
  if (nArgs > arg && isdigit( ppArgs[ arg ][ 0 ] ))
    {
      if (ppArgs[ arg ][ 0 ] == '0')
	{
	  ppArgs[ arg ][ 0 ] = '\0';
	}
      else
	{
	  ppArgs[ arg ] = "-c";
	}
    }

  /* The fourth argument, is a numeric boolean, indicating if we should enable the debug menu */
      
  if (nArgs > (arg + 2) && isdigit( ppArgs[ (arg + 2) ][ 0 ] ))
    {
      if (ppArgs[ (arg + 2) ][ 0 ] == '0')
	{
	  ppArgs[ (arg + 2) ][ 0 ] = '\0';
	}
      else
	{
	  ppArgs[ (arg + 2) ] = "-D";
	}
    }
      
  /* The third argument, is the file descriptor for the connection to the I/O server */
  /* NB/ This argument must be processed after arguments 2 and 4 */
  
  if (nArgs > (arg + 1) && isdigit( ppArgs[ (arg + 1) ][ 0 ] ))
    {
      if (ppArgs[ arg ][ 0 ] == '\0')
	{
	  /* use the blank second argument */
	  
	  ppArgs[ arg ] = "-p";
	}
      else if (nArgs > (arg + 2) && ppArgs[ (arg + 2) ][ 0 ] == '\0') 
	{
	  /* use blank fourth argument */

	  ppArgs[ (arg + 2) ] = ppArgs[ (arg + 1) ];
	  ppArgs[ (arg + 1) ] = "-p";
	}
      else
	{
	  char *	pString;

	  
	  /* ho hum, we are forced to create a new string */
	  
	  pString = (char *)malloc( strlen( ppArgs[ (arg + 1) ] ) + 3 );
	  
	  if (pString == NULL)
	    {
	      fail( "out of memory duplicating argument string" );
	    }
	  
	  strcpy( pString, "-p" );
	  
	  strcat( pString, ppArgs[ (arg + 1) ] );
	  
	  ppArgs[ (arg + 1) ] = pString;
	}
    }
      
  return;
  
} /* PreProcessArguments */

/*}}}*/
/*{{{  ProcessArguments    */

/*
 * FUNCTION: ProcessArguments
 *
 * ARGUMENTS:
 *	int	nArgs		- number of arguments passed in
 *	char **	ppArgs		- vector of argument strings
 *
 * DESCRIPTION:
 *	Processes the aruments from the command line that
 *	control the behaviour of this program, (as opposed
 *	to those command line arguments that are intended for
 *	the X toolkit).
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE void
ProcessArguments
#ifdef __STDC__
  ( int nArgs, char ** ppArgs )
#else
  ( nArgs, ppArgs )
int	   nArgs;
char **	   ppArgs;
#endif /* __STDC */
{
  int	inform = 0;
  int	stop   = 0;
  

  while (nArgs--)
    {
      char *	pArg = *++ppArgs;


      if (*pArg == '-')
	{
	  switch (*++pArg)
	    {
	    case 'h':
	    case '?':
	      inform = 1;
	      stop   = 1;
	      nArgs  = 0;
	      break;

	    case 'c':
	      console_window = TRUE;
	      break;

	    case 'D':
	      show_debug = ~show_debug;
	      break;

	    case 'p':
	      if (*++pArg == '\0')
		{
		  from_io_server = atoi( pArg = *++ppArgs );
		  --nArgs;
		}
	      else
		{
		  from_io_server = atoi( pArg );
		}

	      if (from_io_server == 0)
		{
		  debug( "URG - bad -p option!" );
		}

	      /* if we have a pipe connection then we must write on it as well.  This is a bug */

	      to_io_server = from_io_server;
	      
	      break;
	      
	    case 't':
	      if (*++pArg == '\0')
		{
		  to_io_server = atoi( pArg = *++ppArgs );
		  --nArgs;
		}
	      else
		{
		  to_io_server = atoi( pArg );
		}
	      break;
	      
	    case 'f':
	      if (*++pArg == '\0')
		{
		  from_io_server = atoi( pArg = *++ppArgs );
		  --nArgs;
		}
	      else
		{
		  from_io_server = atoi( pArg );
		}
	      break;

	    case 'z':
	      if (*++pArg == '\0')
		{
		  siphon = atoi( pArg = *++ppArgs );
		  --nArgs;
		}
	      else
		{
		  siphon = atoi( pArg );
		}
	      
	      break;

	    default:
	      warn( "unknown command line switch %s", pArg );
	      break;
	    }
	}
      else if (*pArg != '\0')	/* ignore blank arguments - they have been placed here by the pre-processor */
	{
	  warn( "unknown command line argument '%s' - ignored", pArg );

	  inform = 1;
	}
    }

  if (inform)
    {
      usage();
    }

  if (stop)
    {
      exit( EXIT_FAILURE );
    }
  
  return;
  
} /* ProcessArguments */

/*}}}*/
/*{{{  HandleIOServerInput */

/*
 * FUNCTION: HandleIOServerInput
 *
 * ARGUMENTS:
 * 	unsigned char *		pText		- buffer containing text to be processed
 *	int			iLength		- length of text in buffer
 *	int			iFd		- connection number for output
 *	Bool			bPassOn		- TRUE iff commands should be passed on
 *	Bool			bProcesssCtrls	- TRUE iff control characters should be processed
 *
 * DESCRIPTION:
 *	This function handles input received from the I/O server.
 *	Most input is past straight on, but special commands are
 *	intercepted and interpreted here.
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
HandleIOServerInput
#ifdef __STDC__
  ( unsigned char * pText, int iLength, int iFd, Bool bPassOn, Bool bProcessCtrls )
#else
  ( pText, iLength, iFd, bPassOn, bProcessCtrls )
unsigned char *	   pText;
int		   iLength;
int		   iFd;
Bool		   bPassOn;
Bool		   bProcessCtrls;
#endif
{
  unsigned char *	pStart;
  unsigned char *	pBuf;
  

  if (bPassOn)
    {
      (void) SafeWrite( iFd, (char *)pText, iLength );

      /* look for a kill code and die if necessary */
      
      while (iLength--)
	{
	  if (*pText++ == FUNCTION_CODE &&
	      *pText   == WINDOW_KILL    )
	    {
	      exit( EXIT_SUCCESS );
	    }
	}
    }
  else
    {
      /* scan input looking for commands */
      
      for (pBuf = pStart = pText;
	   iLength > 0;
	   pBuf++, iLength--)
	{
	  if (*pBuf == FUNCTION_CODE)
	    {
	      if (pBuf > pStart)
		{
		  (void) SafeWrite( iFd, (char *) pStart, pBuf - pStart );
		  
		  pStart = pBuf;
		}
	      
	      switch (pBuf[1])
		{
		case WINDOW_KILL:
		  exit( EXIT_SUCCESS );
		  break;
		  
		case WINDOW_PANEL:
		  if (!siphon && console_window)
		    PanelInput( pBuf[ 2 ], pBuf[  3 ] );
		  break;
		  
		default:
		  warn( "unknown window protocol byte: %x", pBuf[ 1 ] );
		  break;
		}
	      
	      /* all command functions occupy 4 bytes, so update pointers */
	      
	      pBuf    += 3;
	      pStart  += 4;
	      iLength -= 3;
	    }
	}
  
      /* finally transmit remainder of message */
  
      if (pBuf > pStart)
	{
	  (void) SafeWrite( iFd, (char *) pStart, pBuf - pStart );
	}
    }
  
  return;
  
} /* HandleIOServerInput */

/*}}}*/
/*{{{  CopyText            */

/*
 * FUNCTION: CopyText
 *
 * ARGUMENTS:
 *	int	iFromId			- File descriptor of source
 *	int	iToId			- File descriptor of sink
 *	Bool	bProcessInput		- TRUE if input should be processed
 *	Bool	bProcessControls	- TRUE if control characters in input should be processed
 *
 * DESCRIPTION:
 * 	Copies text from source to sink
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
CopyText
#ifdef __STDC__
  ( int iFromId, int iToId, Bool bProcessInput, Bool bProcessControls )
#else
  ( iFromId, iToId, bProcessInput, bProcessControls )
int	iFromId;
int 	iToId;
Bool	bProcessInput;
Bool	bProcessControls;
#endif
{
  unsigned char 	aBuffer[ IN_BUF_SIZE ];
  int			got;

  
  /* read data from source */

  got = read( iFromId, (char *)aBuffer, sizeof (aBuffer) );

  if (got < 0)
    {
#if 0
      fail( "read from source failed, errno = %d - aborting", errno );
#else
      exit(EXIT_FAILURE);
#endif
    }

  if (got == 0)
    {
#if 0
      fail( "read from source timed out, even though data should be available!" );
#else
      exit(EXIT_FAILURE);
#endif
    }

  if (bProcessInput)
    {
      HandleIOServerInput( aBuffer, got, iToId, FALSE, bProcessControls );
    }
  else
    {
      (void) SafeWrite( iToId, (char *)aBuffer, got );
    }
  
  return;
  
} /* CopyText */

/*}}}*/
/*{{{  SendWindowSize      */

/*
 * FUNCTION: SendWindowSize
 *
 * ARGUMENTS:
 *	int	iFd		- File descriptor used to send message
 *	int	iRows		- Number of rows on window
 *	int	iColumns	- Number of columns on window
 *
 * DESCRIPTION:
 *	Sends the current size of the terminal to the I/O server.
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
SendWindowSize
#ifdef __STDC__
( int iFd, int iRows, int iColumns )
#else
( iFd, iRows, iColumns )
int iFd;
int iRows;
int iColumns;
#endif
{
  char		aMessage[ 4 ];
      

  aMessage[ 0 ] = FUNCTION_CODE;
  aMessage[ 1 ] = WINDOW_SIZE;
  aMessage[ 2 ] = iRows;
  aMessage[ 3 ] = iColumns;

  if (!SafeWrite( iFd, aMessage, sizeof (aMessage) / sizeof (aMessage[0]) ))
    {
      fail( "Unable to send window size to IO server" );      
    }  
  
  return;
  
} /* SendWindowSize */

/*}}}*/
/*{{{  StartXterm          */

/*
 * FUNCTION: StartXterm
 *
 * ARGUMENTS:
 *       None.
 *
 * DESCRIPTION:
 *	Starts an X term session running.
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
StartXterm
#ifdef __STDC__
  ( void )
#else
  ()
#endif
{
  int			sock;
  struct sockaddr_in	sockAddr;
  int			iAddrLen = sizeof (sockAddr);
  int			port     = PORT;
  

  if ((sock = socket( AF_INET, SOCK_STREAM, 0 )) < 0)
    {
      fail( "unable to create socket\n" );
    }

  if (setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &iAddrLen, sizeof (iAddrLen) ))
    {
      fail( "unable to set reuse of socket address" );
    }

  sockAddr.sin_family      = AF_INET;
  sockAddr.sin_addr.s_addr = INADDR_ANY;

  do
    {
      port++;
      
      sockAddr.sin_port = htons( port );
    }
  while (bind( sock, (struct sockaddr *)&sockAddr, iAddrLen ) < 0 && port < MAXPORT);

  if (port >= MAXPORT)
    {
      fail( "unable to find free port to bind socket" );
    }
  
  if (listen( sock, 5 ) < 0)
    {
      fail( "unable to set up listening" );
    }
  
  if (vfork() == 0)
    {
      char 	aServerArg[ 8 ];
      char *	pProgram;
      

      if ((pProgram = getenv( "SERVERXAW_TERMINAL_EMULATOR" )) == NULL)
	{
	  pProgram = "xterm";
	}
      
      close( sock );

      sprintf( aServerArg, "-z%d", port );
      
      execlp( pProgram, pProgram, "-T", TitleName, "-e", ProgName, aServerArg, NULL );

      fail( "could not run %s", pProgram );
    }

  if ((to_xterm = from_xterm = accept( sock, (struct sockaddr *)&sockAddr, &iAddrLen )) < 0)
    {
      fail( "unable to accept connection" );
    }

  return;

} /* StartXterm */

/*}}}*/
/*{{{  Siphon              */

/*
 * FUNCTION: Siphon
 *
 * ARGUMENTS:
 *      int 	iFromIO			- connection number from IO server
 *	int	iToIO			- connection number to   IO server
 *	int	iFromXterm		- connection number from Xterm
 *	int	iToXterm		- connection number to   Xterm
 *	Bool	bProcessControls	- TRUE if control characters should be processed	
 *
 * DESCRIPTION:
 * 	Copies data from IO server to X term and vice versa
 *	Interprets control messages from IO server.
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
Siphon
#ifdef __STDC__
  ( int iFromIO, int iToIO, int iFromXterm, int iToXterm, Bool bProcessControls )
#else
  ( iFromIO, iToIO, iFromXterm, iToXterm, bProcessControls )
int	iFromIO;
int	iToIO;
int	iFromXterm;
int	iToXterm;
Bool	bProcessControls;
#endif
{
  int			iReadMask;
  int			iMaxFd;
  

  iReadMask = (1 << iFromIO) | (1 << iFromXterm);
  iMaxFd    = iFromIO > iFromXterm ? iFromIO : iFromXterm;

  for (;;)
    {
      int	iResult;
      int	iCopyReadMask = iReadMask;

      
      iResult = select( iMaxFd + 1, &iCopyReadMask, NULL, NULL, NULL );

      if (iResult <= 0)
	{
	  fail( "select failed, res = %d", iResult );
	}
      
      if (iCopyReadMask & (1 << iFromXterm))
	{
	  CopyText( iFromXterm, iToIO, FALSE, FALSE );

	  iCopyReadMask &= ~(1 << iFromXterm);
	}

      if (iCopyReadMask & (1 << iFromIO))
	{
	  CopyText( iFromIO, iToXterm, TRUE, bProcessControls );
	  
	  iCopyReadMask &= ~(1 << iFromIO);
	}
      
      if (iCopyReadMask)
	{
	  fail( "input received from unknown source %x", iCopyReadMask );
	}      
    }
  
} /* Siphon */

/*}}}*/
/*{{{  CreateSiphon        */

/*
 * FUNCTION: CreateSiphon
 *
 * ARGUMENTS:
 *       None.
 *
 * DESCRIPTION:
 *	Connects to the other end of the siphon and then starts
 *	the siphon process
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
CreateSiphon
#ifdef __STDC__
  ( void )
#else
  ()
#endif
{
  char			aHostName[ 100 ];	/* XXX */
  struct hostent *	pHostent;
  struct sockaddr_in	sockAddr;
  struct termio		sTermio;
  struct winsize	sWinsize;
  int			iAddrLen = sizeof (sockAddr);
  int			tries    = NUM_RETRIES;
  int			iDataSocket;
  

  sockAddr.sin_family = AF_INET;
  sockAddr.sin_port   = htons( siphon );

  if (gethostname( aHostName, sizeof (aHostName) ))
    {
      fail( "Unable to get name of host" );
    }
  
  if ((pHostent = gethostbyname( aHostName )) == NULL)
    {
      fail( "unable to locate host entry" );
    }

  if (pHostent->h_addrtype != AF_INET)
    {
      fail( "not internet host name" );
    }

  bcopy( (char *)pHostent->h_addr, (char *)&sockAddr.sin_addr, sizeof (sockAddr.sin_addr) );

  /* make data connnection */
  
  for (;;)
    {
      if ((iDataSocket = socket( AF_INET, SOCK_STREAM, 0 )) < 0)
	{
	  fail( "unable to create data socket\n" );
	}

      errno = 0;
      
      if (connect( iDataSocket, (struct sockaddr *)&sockAddr, iAddrLen ) < 0)
	{
	  close( iDataSocket );

	  if (tries-- > 0)
	    {
	      sleep( 1 );
	    }
	  else
	    {
	      fail( "unable to connect to %s", aHostName );
	    }
	}
      else
	{
	  break;
	}
    }

  /* set non-blocking input */
  
#ifdef FIOSNBIO
    {
      int arg = 1;
      
      ioctl( iDataSocket, FIOSNBIO, &arg );
    }
#else
  (void) fcntl( iDataSocket, F_SETFL, FNDELAY );
#endif /* FIOSNBIO */

  /* turn off canonical input processing and output processing */

  ioctl( 0, TCGETA, (caddr_t) &sTermio );

  sTermio.c_iflag &= ~( IXON | IXOFF );
  sTermio.c_iflag |=    IGNBRK | IGNPAR | INLCR;
  sTermio.c_lflag &= ~( ICANON | ECHO | ISIG | ECHOE | ECHOK | ECHONL );
  sTermio.c_cc[ VMIN  ] = 1;
  sTermio.c_cc[ VTIME ] = 0;
  
  ioctl( 0, TCSETAW, (caddr_t) &sTermio );
  
  ioctl( 1, TCGETA, (caddr_t) &sTermio );

  sTermio.c_oflag &= ~( ONLCR | OCRNL | ONOCR | ONLRET );
  sTermio.c_lflag &= ~( ICANON | ECHO | ISIG | ECHOE | ECHOK | ECHONL );
  sTermio.c_cc[ VMIN  ] = 1;
  sTermio.c_cc[ VTIME ] = 0;
  
  ioctl( 1, TCSETAW, (caddr_t) &sTermio );

  /* Send the window size back to the IO server */

  ioctl( 0, TIOCGWINSZ, (caddr_t) &sWinsize );
  
  SendWindowSize( iDataSocket, sWinsize.ws_row, sWinsize.ws_col );
  
  Siphon( iDataSocket, iDataSocket, 0, 1, TRUE );

  return;
  
} /* CreateSiphon */

/*}}}*/

/*{{{  WIDGETS             */

/*{{{  WIDGET Variables    */

PRIVATE XtAppContext	serverContext;
PRIVATE Widget		rootWidget;
PRIVATE Widget		loggerWidget;
PRIVATE Widget		debugWidget;
PRIVATE Cardinal	nXtArgs;
PRIVATE Arg		pXtArgs[ NUM_ARGS ];

/*}}}*/

/*{{{  RebootHandler       */

/*
 * FUNCTION: RebootHandler
 *
 * ARGUMENTS:
 *	Widget		w		- the widget that was activated
 *	XtPointer	pCallData	- data created when callback was added to widget
 *	XtPointer	pClientData	- data passed by toolkit to callback
 *
 * DESCRIPTION:
 *	Handles the user pressing the reboot button
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE void
RebootHandler
#ifdef __STDC__
  ( Widget w, XtPointer call_data, XtPointer client_data )
#else
  ( w, call_data, client_data )
Widget		w;
XtPointer	call_data;
XtPointer	client_data;
#endif /* __STDC */
{
  if (to_io_server)
    {
      char	aMessage[ 4 ];


      aMessage[ 0 ] = FUNCTION_CODE;
      aMessage[ 1 ] = WINDOW_PANEL;
      aMessage[ 2 ] = WIN_REBOOT;
      aMessage[ 3 ] = 0;

      (void) SafeWrite( to_io_server, aMessage, sizeof (aMessage) );
    }

  return;
  
} /* RebootHandler */

/*}}}*/
/*{{{  StatusHandler       */

/*
 * FUNCTION: StatusHandler
 *
 * ARGUMENTS:
 *	Widget		w		- the widget that was activated
 *	XtPointer	pCallData	- data created when callback was added to widget
 *	XtPointer	pClientData	- data passed by toolkit to callback
 *
 * DESCRIPTION:
 *	Handles the user pressing the Status button
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE void
StatusHandler
#ifdef __STDC__
  ( Widget w, XtPointer pCallData, XtPointer pClientData )
#else
  ( w, pCallData, pClientData )
Widget		w;
XtPointer	pCallData;
XtPointer	pClientData;
#endif /* __STDC */
{
  if (to_io_server)
    {
      char	aMessage[ 4 ];


      aMessage[ 0 ] = FUNCTION_CODE;
      aMessage[ 1 ] = WINDOW_PANEL;
      aMessage[ 2 ] = WIN_STATUS;
      aMessage[ 3 ] = 0;

      (void) SafeWrite( to_io_server, aMessage, sizeof (aMessage) );
    }
  
  return;
  
} /* StatusHandler */

/*}}}*/
/*{{{  ExitHandler         */

/*
 * FUNCTION: ExitHandler
 *
 * ARGUMENTS:
 *	Widget		w		- the widget that was activated
 *	XtPointer	pCallData	- data created when callback was added to widget
 *	XtPointer	pClientData	- data passed by toolkit to callback
 *
 * DESCRIPTION:
 *	Handles the user pressing the Exit button
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE void
ExitHandler
#ifdef __STDC__
  ( Widget w, XtPointer pCallData, XtPointer pClientData )
#else
  ( w, pCallData, pClientData )
Widget		w;
XtPointer	pCallData;
XtPointer	pClientData;
#endif /* __STDC */
{
  if (to_io_server)
    {
      char	aMessage[ 4 ];


      aMessage[ 0 ] = FUNCTION_CODE;
      aMessage[ 1 ] = WINDOW_PANEL;
      aMessage[ 2 ] = WIN_EXIT;
      aMessage[ 3 ] = 0;

      (void) SafeWrite( to_io_server, aMessage, sizeof (aMessage) );
      (void) SafeWrite( to_xterm,     aMessage, sizeof (aMessage) );
    }

  exit( EXIT_SUCCESS );
  
  return;
  
} /* ExitHandler */

/*}}}*/
/*{{{  LogHandler          */

/*
 * FUNCTION: LogHandler
 *
 * ARGUMENTS:
 *	Widget		w		- the widget that was activated
 *	XtPointer	PCallData	- data created when callback was added to widget
 *	XtPointer	pClientData	- data passed by toolkit to callback
 *
 * DESCRIPTION:
 *	Handles the user pressing the Log button
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE void
LogHandler
#ifdef __STDC__
  ( Widget w, XtPointer pCallData, XtPointer pClientData )
#else
  ( w, pCallData, pClientData )
Widget		w;
XtPointer	pCallData;
XtPointer	pClientData;
#endif /* __STDC */
{
  char	aMessage[ 4 ];

  
  /* NB/ numbering in this switch statement MUST match order in menu creation in InitXt() */

  aMessage[ 0 ] = FUNCTION_CODE;
  aMessage[ 1 ] = WINDOW_PANEL;
  
  resetArgs_();
  
  switch ((int)pCallData)
    {
    case 0:
      aMessage[ 2 ] = WIN_LOG_SCREEN;
      setArg_( XtNlabel, "Logger -\nScreen" );
      break;
      
    case 1:
      aMessage[ 2 ] = WIN_LOG_FILE;
      setArg_( XtNlabel, "Logger -\nFile" );
      break;
      
    case 2:
      aMessage[ 2 ] = WIN_LOG_BOTH;
      setArg_( XtNlabel, "Logger -\nBoth" );
      break;
      
    default:
      warn( "unknown log menu item number 0x%p", pCallData );
      break;
    }

  if (nXtArgs)
    XtSetValues( loggerWidget, pXtArgs, nXtArgs );

  aMessage[ 3 ] = 0;

  if (to_io_server)
    {
      (void) SafeWrite( to_io_server, aMessage, sizeof (aMessage) );
    }
  
  return;
  
} /* LogHandler */

/*}}}*/
/*{{{  DebugHandler        */

/*
 * FUNCTION: DebugHandler
 *
 * ARGUMENTS:
 *	Widget		w		- the widget that was activated
 *	XtPointer	pCallData	- data created when callback was added to widget
 *	XtPointer	pClientData	- data passed by toolkit to callback
 *
 * DESCRIPTION:
 *	Handles the user pressing the Debug button
 *
 * RETURNS:
 * 	Nothing.
 */

PRIVATE void
DebugHandler
#ifdef __STDC__
  ( Widget w, XtPointer pCallData, XtPointer pClientData )
#else
  ( w, pCallData, pClientData )
Widget		w;
XtPointer	pCallData;
XtPointer	pClientData;
#endif /* __STDC */
{
  static Boolean	aState[ 19 ];
  static Pixel		onPixel  = -1;
  static Pixel		offPixel = -1;
  char			aMessage[ 4 ];

  
  if (!console_window || !show_debug)	/* only console windows have a debug menu */
    return;

  if (onPixel == -1)
    {
      XColor	red;
      XColor	exact;


      resetArgs_();
      setArg_( XtNforeground, &offPixel );

      XtGetValues( w, pXtArgs, nXtArgs );
      
      (void) XAllocNamedColor( XtDisplay( rootWidget ),
			      XDefaultColormap( XtDisplay( rootWidget ), 0 ),
			      "red",
			      &red, &exact );

      onPixel = red.pixel;
    }
  
  aMessage[ 0 ] = FUNCTION_CODE;
  aMessage[ 1 ] = WINDOW_PANEL;
  
  /* NB/ numbering in this switch statement MUST match order in menu creation in BuildWidgets() */
  
  switch ((int)pCallData)
    {
    case 0:  aMessage[ 2 ] = WIN_MEMORY;      break;
    case 1:  aMessage[ 2 ] = WIN_RECONF;      break;
    case 2:  aMessage[ 2 ] = WIN_MESSAGES;    break;
    case 3:  aMessage[ 2 ] = WIN_SEARCH;      break;
    case 4:  aMessage[ 2 ] = WIN_OPEN;        break;
    case 5:  aMessage[ 2 ] = WIN_CLOSE;       break;
    case 6:  aMessage[ 2 ] = IOWIN_NAME;      break;
    case 7:  aMessage[ 2 ] = WIN_READ;        break;
    case 8:  aMessage[ 2 ] = WIN_BOOT;        break;
    case 9:  aMessage[ 2 ] = WIN_KEYBOARD;    break;
    case 10: aMessage[ 2 ] = WIN_INIT;        break;
    case 11: aMessage[ 2 ] = WIN_WRITE;       break;
    case 12: aMessage[ 2 ] = WIN_QUIT;        break;
    case 13: aMessage[ 2 ] = WIN_GRAPHICS;    break;
    case 14: aMessage[ 2 ] = WIN_TIMEOUT;     break;
    case 15: aMessage[ 2 ] = WIN_OPENREPLY;   break;
    case 16: aMessage[ 2 ] = WIN_FILEIO;      break;
    case 17: aMessage[ 2 ] = WIN_DELETE;      break;
    case 18: aMessage[ 2 ] = WIN_DIRECTORY;   break;

    default:
      warn( "unknown debug menu entry 0x%p", pCallData );
      break;
    }

  if (aState[ (int)pCallData ] == TRUE)
    {
      aMessage[ 3 ] = WIN_OFF;

      aState[ (int)pCallData ] = FALSE;
      
      resetArgs_();
      setArg_( XtNforeground, offPixel );

      XtSetValues( w, pXtArgs, nXtArgs );
    }
  else
    {
      aMessage[ 3 ] = WIN_ON;
      
      aState[ (int)pCallData ] = TRUE;

      resetArgs_();
      setArg_( XtNforeground, onPixel );

      XtSetValues( w, pXtArgs, nXtArgs );
    }

  if (to_io_server)
    {
      (void) SafeWrite( to_io_server, aMessage, sizeof (aMessage) );
    }
  
  return;
  
} /* DebugHandler */

/*}}}*/
/*{{{  AddButton           */

/*
 * FUNCTION: AddButton
 *
 * ARGUMENTS:
 *	char *		pName 		- name of the button to be created
 *	Widget		parentWidget	- parent widget for the button
 *	XtCallbackProc	pCallback	- routine to be called when button is pressed
 *
 * DESCRIPTION:
 *	Creates a new (X toolkit) button and places it inside
 *	parent widget provided.
 *
 * RETURNS:
 * 	The new button widget or 0.
 */

PRIVATE Widget
AddButton
#ifdef __STDC__
  ( char * pName, Widget parentWidget, XtCallbackProc pCallback )
#else
  ( pName, parentWidget, pCallback )
char *		pName;
Widget 		parentWidget;
XtCallbackProc	pCallback;
#endif
{
  Widget	buttonWidget;


  resetArgs_();
  setArg_( XtNshowGrip,   False );
  setArg_( XtNskipAdjust, True );

  buttonWidget = XtCreateManagedWidget( pName, commandWidgetClass, parentWidget, pXtArgs, nXtArgs );

  XtAddCallback( buttonWidget, XtNcallback, pCallback, NULL );

  return buttonWidget;
  
} /* AddButton */

/*}}}*/
/*{{{  AddMenu             */

/*
 * FUNCTION: AddMenu
 *
 * ARGUMENTS:
 *	char *		pName 		- name of the menu to be created
 *	Widget		parentWidget	- parent widget for the menu
 *	XtCallbackProc	pCallback	- menu handler function
 *	...				- names of menu entries.
 *	NULL				- terminator for list of names
 *
 * DESCRIPTION:
 *	Creates a new (X toolkit) menu and button and places them inside
 *	parent widget provided.  The names passed to this function
 *	are entered sequentially into the menu, with a unique,
 *	incrementing index starting from 0.  Whenever a entry on the
 *	menu is selected by the user the menu handler function will
 *	be called with index number of the entry passed as the data.
 *
 * RETURNS:
 * 	The new menu button or 0.
 */

PRIVATE Widget
AddMenu
#ifdef __STDC__
  (char * pName, Widget parentWidget, XtCallbackProc pCallback, ... )
#else
  ( pName, parentWidget, pCallback, va_alist )
char *		pName;
Widget 		parentWidget;
XtCallbackProc	pCallback;
va_dcl
#endif
{
  Widget	menuButtonWidget;
  Widget	menuWidget;
  Widget	menuItemWidget;
  va_list	args;
  int		index;
  

  resetArgs_();
  setArg_( XtNshowGrip,   False );
  setArg_( XtNskipAdjust, True );
  setArg_( XtNmenuName,   pName );
  setArg_( XtNshapeStyle, XmuShapeRoundedRectangle );
  
  /* create the button for the menu */
  
  menuButtonWidget = XtCreateManagedWidget( pName, menuButtonWidgetClass, parentWidget, pXtArgs, nXtArgs );

  /* create the menu */
  
  menuWidget = XtCreatePopupShell( pName, simpleMenuWidgetClass, menuButtonWidget, NULL, ZERO );
  
  /* create the entries in the menu */

#ifdef __STDC__
  va_start( args, pCallback );
#else
  va_start( args );
#endif

  index = 0;
  
  for (;;)
    {
      char *	pItemName;


      /* get the name of the entry */
      
      pItemName = va_arg( args, char * );
      
      if (pItemName == NULL)
	break;

      /* create an entry in the menu with that name */
      
      menuItemWidget = XtCreateManagedWidget( pItemName, smeBSBObjectClass, menuWidget, NULL, ZERO );

      /* add a call to the menu handler with a datum indicating which entry was selected */
      
      XtAddCallback( menuItemWidget, XtNcallback, pCallback, (XtPointer) index );

      ++index;
    }

  /* tidy up */
  
  va_end( args );
  
  return menuButtonWidget;
  
} /* AddMenu */

/*}}}*/
/*{{{  InitXt              */

PRIVATE String		aServerResources[] =
  {
#if 0
    "*font:		fixed",
#else
    "*font:		-adobe-helvetica-bold-r-normal--*-100-*-*-*-*-*-*",
#endif
    NULL,
  };

struct _resources
  {
    int			dummy;
  }
appResources;

#define offset( field )	XtOffset( struct _resources *, field )

PRIVATE XtResource	aResources[] =
  {
    { "dummy", "Dummy", XtRInt, sizeof (int), offset (dummy), XtRString, "1" },
  };

#undef offset

/*
 * FUNCTION: InitXt
 *
 * ARGUMENTS:
 *	int	nArgs		- number of arguments passed in
 *	char **	ppArgs		- vector of argument strings
 *
 * DESCRIPTION:
 *	Initialises the X toolkit.  Builds the root widget,
 *	As a side effect it also processes the command line
 *	passed to the program and removes any arguments
 *	that are specific to the toolkit.
 *
 * RETURNS:
 * 	The new value of nArgs, after all of the toolkit
 *	specific arguments have been removed.
 */

PRIVATE int
InitXt
#ifdef __STDC__
  ( Cardinal nArgs, char ** ppArgs )
#else
  ( nArgs, ppArgs )
Cardinal   nArgs;
char **	   ppArgs;
#endif
{
  resetArgs_();
  setArg_( XtNname, TitleName );
  
  /* create the root widget and initialise the X toolkit */
  
  rootWidget = XtAppInitialize( &serverContext,
			       ProgName,
			       NULL 		/* command line options table */,
			       0		/* number of options */,
#ifdef __HELIOS
			       (int *)&nArgs,
#else
			       &nArgs,
#endif
			       ppArgs,
			       aServerResources,
			       pXtArgs,
			       nXtArgs );
  
  /* get hold of any resources specified by the user */
  
  XtGetApplicationResources( rootWidget,
			    &appResources,
			    aResources, XtNumber( aResources ),
			    NULL, 0 );
  
  XtSetValues( rootWidget, pXtArgs, nXtArgs );
  
  /* finished, return adjusted argument count */
  
  return nArgs;
  
} /* InitXt */

/*}}}*/
/*{{{  PanelInput          */

/*
 * FUNCTION: PanelInput
 *
 * ARGUMENTS:
 * 	unsigned char	panelEvent	- Protocol byte indicating which widget to update.
 *	unsigned char	panelValue	- Protocol byte indicating new value for selected widget.
 *
 * DESCRIPTION:
 *	This function interprets protocol messages from the I/O server
 *	and sets values on the control panel as appropriate.
 *
 * RETURNS:
 * 	None
 */

PRIVATE void 
PanelInput
#ifdef __STDC__
  ( unsigned char panelEvent, unsigned char panelValue )
#else
  ( panelEvent, panelValue )
unsigned char panelEvent;
unsigned char panelValue;
#endif
{
  switch (panelEvent)
    {
    case WIN_LOG_SCREEN:	LogHandler( loggerWidget,   (XtPointer)0, (XtPointer)&panelValue ); break;
    case WIN_LOG_FILE:		LogHandler( loggerWidget,   (XtPointer)1, (XtPointer)&panelValue ); break;
    case WIN_LOG_BOTH:		LogHandler( loggerWidget,   (XtPointer)2, (XtPointer)&panelValue ); break;
    case WIN_MEMORY:		DebugHandler( debugWidget,  (XtPointer)0, (XtPointer)&panelValue ); break; 
    case WIN_RECONF:		DebugHandler( debugWidget,  (XtPointer)1, (XtPointer)&panelValue ); break; 
    case WIN_MESSAGES:		DebugHandler( debugWidget,  (XtPointer)2, (XtPointer)&panelValue ); break; 
    case WIN_SEARCH:		DebugHandler( debugWidget,  (XtPointer)3, (XtPointer)&panelValue ); break; 
    case WIN_OPEN:		DebugHandler( debugWidget,  (XtPointer)4, (XtPointer)&panelValue ); break; 
    case WIN_CLOSE:		DebugHandler( debugWidget,  (XtPointer)5, (XtPointer)&panelValue ); break; 
    case IOWIN_NAME:		DebugHandler( debugWidget,  (XtPointer)6, (XtPointer)&panelValue ); break; 
    case WIN_READ:		DebugHandler( debugWidget,  (XtPointer)7, (XtPointer)&panelValue ); break; 
    case WIN_BOOT:		DebugHandler( debugWidget,  (XtPointer)8, (XtPointer)&panelValue ); break; 
    case WIN_KEYBOARD:		DebugHandler( debugWidget,  (XtPointer)9, (XtPointer)&panelValue ); break; 
    case WIN_INIT:		DebugHandler( debugWidget, (XtPointer)10, (XtPointer)&panelValue ); break; 
    case WIN_WRITE:		DebugHandler( debugWidget, (XtPointer)11, (XtPointer)&panelValue ); break; 
    case WIN_QUIT:		DebugHandler( debugWidget, (XtPointer)12, (XtPointer)&panelValue ); break; 
    case WIN_GRAPHICS:		DebugHandler( debugWidget, (XtPointer)13, (XtPointer)&panelValue ); break; 
    case WIN_TIMEOUT:		DebugHandler( debugWidget, (XtPointer)14, (XtPointer)&panelValue ); break; 
    case WIN_OPENREPLY:		DebugHandler( debugWidget, (XtPointer)15, (XtPointer)&panelValue ); break; 
    case WIN_FILEIO:		DebugHandler( debugWidget, (XtPointer)16, (XtPointer)&panelValue ); break; 
    case WIN_DELETE:		DebugHandler( debugWidget, (XtPointer)17, (XtPointer)&panelValue ); break; 
    case WIN_DIRECTORY:		DebugHandler( debugWidget, (XtPointer)18, (XtPointer)&panelValue ); break; 
    default:
      warn( "unknown window panel protocol byte: %x", panelEvent );
      break;
    }

  return;
  
} /* PanelInput */

/*}}}*/
/*{{{  FromIOServer        */

/*
 * FUNCTION: FromIOServer
 *
 * ARGUMENTS:
 *	XtPointer   pClientData		- Data provided by program when callback was created.
 *	int *       pSource		- Pointer to the file descriptor containing input.
 *	XtInputId * pInputId		- Pointer to the input Id allocated when the callback was created.
 *
 * DESCRIPTION:
 *      Xt Callback routine.  
 *	This function handles input received from the I/O server.
 *	Most input is past straight on to Xterm, but
 *	special commands are intercepted and interpreted here.
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
FromIOServer
#ifdef __STDC__
  ( XtPointer pClientData, int * pSource, XtInputId * pInputId )
#else
  ( pClientData, pSource, pInputId )
XtPointer	 pClientData;
int *		 pSource;
XtInputId *	 pInputId;
#endif
{
  unsigned char 	aBuffer[ IN_BUF_SIZE ];
  int			got;
  unsigned char *	pBuf;
  unsigned char *	pStart;
  

  /* read data from I/O server */

  got = read( *pSource, (char *)aBuffer, sizeof (aBuffer) );

  if (got < 0)
    {
      fail( "read from I/O server failed, errno = %d - aborting", errno );
    }

  if (got == 0)
    {
      fail( "read from I/O server timed out, even though data should be available!" );
    }

  HandleIOServerInput( aBuffer, got, to_xterm, TRUE, FALSE );
  
  return;
  
} /* FromIOServer */

/*}}}*/
/*{{{  FromXterm           */

/*
 * FUNCTION: FromXterm
 *
 * ARGUMENTS:
 *	XtPointer   pClientData		- Data provided by program when callback was created.
 *	int *       pSource		- Pointer to the file descriptor containing input.
 *	XtInputId * pInputId		- Pointer to the input Id allocated when the callback was created.
 *
 * DESCRIPTION:
 *      Xt Callback routine.  
 *	This function handles input received from Xterm.
 *	Most input is past straight on to the IO server.
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
FromXterm
#ifdef __STDC__
  ( XtPointer pClientData, int * pSource, XtInputId * pInputId )
#else
  ( pClientData, pSource, pInputId )
XtPointer	 pClientData;
int *		 pSource;
XtInputId *	 pInputId;
#endif
{
  unsigned char 	aBuffer[ IN_BUF_SIZE ];
  int			got;

  
  /* read data from I/O server */

  got = read( *pSource, (char *)aBuffer, sizeof (aBuffer) );

  if (got < 0)
    {
      fail( "read from Xterm failed, errno = %d - aborting", errno );
    }

  if (got == 0)
    {
      fail( "read from Xterm timed out, even though data should be available!" );
    }

  if (!SafeWrite( to_io_server, (char *) aBuffer, got ))
    {
      fail( "unable to write whole of input from Xterm to IO server" );
    }
  
  return;
  
} /* FromXterm */

/*}}}*/
/*{{{  BuildWidgets        */

/*
 * FUNCTION: BuildWidgets
 *
 * ARGUMENTS:
 *       None.
 *
 * DESCRIPTION:
 *	Builds all of the widgets, buttons, menus, etc
 *      necessary to run the program.
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
BuildWidgets
#ifdef __STDC__
  ( void )
#else
  ()
#endif
{
  Display *	pDpy;
  Widget	panelWidget;
  Widget	paneWidget;
  Widget	rebootWidget;
  Widget	statusWidget;
  Widget	exitWidget;
  Pixmap	serverIcon;

  
  /* create an icon for the server */
  
  pDpy = XtDisplay( rootWidget );
  
  serverIcon = XCreateBitmapFromData( pDpy, RootWindow( pDpy, XDefaultScreen( pDpy ) ),
				     ioserv_bits, ioserv_width, ioserv_height );
  
  /* add the icon to the top level widget */
  
  resetArgs_();
  setArg_( XtNiconPixmap, serverIcon );
  
  XtSetValues( rootWidget, pXtArgs, nXtArgs );
  
  /* create a pane widget to hold the buttons */
  
  resetArgs_();
  setArg_( XtNsensitive, True );
  
  paneWidget = XtCreateManagedWidget( "pane", panedWidgetClass, rootWidget, pXtArgs, nXtArgs );
  
  /* now create a panel inside the pane */
  
  resetArgs_();
  setArg_( XtNshowGrip,            False );
  setArg_( XtNsensitive,           True );
  setArg_( XtNinternalBorderWidth, 10 );
  setArg_( XtNorientation,         XtorientHorizontal );
  
  panelWidget = XtCreateManagedWidget( "panel", panedWidgetClass, paneWidget, pXtArgs, nXtArgs );
  
  /* next, create the buttons and menus */
  
  rebootWidget = AddButton( "Reboot", panelWidget, RebootHandler );
  statusWidget = AddButton( "Status", panelWidget, StatusHandler );
  exitWidget   = AddButton( "Exit",   panelWidget, ExitHandler   );
  loggerWidget = AddMenu(   "Logger", panelWidget, LogHandler, "Screen", "File", "Both", NULL );
  
  if (show_debug)
    {
      debugWidget = AddMenu( "Debug",     panelWidget,   DebugHandler,
			    "Resources", "Reconfigure", "Messages",
			    "Search",    "Open",	"Close",
			    "Name",      "Read",	"Boot",
			    "Keyboard",  "Init",	"Write",
			    "Quit",      "Graphics",    "Timeout",
			    "Open Reply","File I/O",    "Delete",
			    "Directory",  NULL );
    }
  
  /* set the correct name for the logger menu */
  
  resetArgs_();
  setArg_( XtNlabel, "Logger -\nBoth" );
  
  XtSetValues( loggerWidget, pXtArgs, nXtArgs );
    
  return;

} /* BuildWidgets */

/*}}}*/
/*{{{  Capture Inputs      */

PRIVATE void
CaptureInputs
#ifdef __STDC__
  (void)
#else
  ()
#endif
{
  long int	 mask;


  mask = XtInputReadMask;
  
  if (from_io_server > 0)
    {
       /* if we are receiving input from the I/O server then add it as an event source */

      (void) XtAppAddInput( serverContext, from_io_server, (XtPointer)mask, FromIOServer, NULL );
    }
         
  (void) XtAppAddInput( serverContext, from_xterm, (XtPointer)mask, FromXterm, NULL );
  
  return;
  
} /* CaptureInputs */

/*}}}*/
/*{{{  PanelSiphon         */

/*
 * FUNCTION: PanelSiphon
 *
 * ARGUMENTS:
 *	int	nArgs		- number of arguments passed in
 *	char **	ppArgs		- vector of argument strings
 *
 * DESCRIPTION:
 * 	Create and run a Siphon with a control panel
 *
 * RETURNS:
 * 	Nothing
 */

PRIVATE void
PanelSiphon
#ifdef __STDC__
  ( int nArgs, char ** ppArgs )
#else
  ( nArgs, ppArgs )
int     nArgs;
char ** ppArgs;
#endif
{
  /* initialise the X toolkit */
  
  (void) InitXt( nArgs, ppArgs );

  /* build the widgets */

  BuildWidgets();

  StartXterm();

  /* add input sources to toolkit's input loop */

  CaptureInputs();
  
#ifndef __HELIOS
  /* reduce our priority, giving precedence to the I/O server */

  nice( 10 );
#endif
      
  /* Display the windows */

  XtRealizeWidget( rootWidget );

  /* Enter Toolkit's main loop */

  DBG( "Entering Toolkit Main Loop" );
  
  XtAppMainLoop( serverContext );

  /* we should never reach here */

  fail( "XtAppMainLoop was terminated abnormally" );

  return;
  
} /* PanelSiphon */

/*}}}*/

/*}}}*/

/*{{{  main                */

/*
 * FUNCTION: main
 *
 * ARGUMENTS:
 *	int	nArgs		- number of arguments passed in
 *	char **	ppArgs		- vector of argument strings
 *
 * DESCRIPTION:
 *	Program's entry point.
 *	Sequentially calls the program's top level functions
 *	then passes control to the X toolkit
 *
 * RETURNS:
 * 	Under normal circumstances this function will not
 *	return, (the program will be aborted via a call to
 *	exit(2)), but in error conditions it can return an
 *	error value.
 */

PUBLIC int
main
#ifdef __STDC__
  ( int nArgs, char ** ppArgs )
#else
  ( nArgs, ppArgs )
int	 	nArgs;
char **		ppArgs;
#endif /* __STDC */
{
  DBG( "Starting" );  
  
  /* extract name of program */
#if 0  
  if ((ProgName = strrchr( ppArgs[ 0 ], '/' )) != NULL)
    {
      ++ProgName;
    }
  else
    {
      ProgName = ppArgs[ 0 ];
    }
#else
	ProgName = ppArgs[0];
#endif

  /* Catch special command line arguments */

  PreProcessArguments( nArgs, ppArgs );

  /* process the remaining arguments */
  
  ProcessArguments( --nArgs, ppArgs );

  /* start a siphon process running */
  
  if (siphon)
    {
      ProgName = "Siphon";

      CreateSiphon();
    }
  else
    {
      if (console_window)
	{
	  PanelSiphon( nArgs, ppArgs );
	}
      else
	{
	  StartXterm();
	  
	  Siphon( from_io_server, to_io_server, from_xterm, to_xterm, FALSE );
	}
    }
    
  return EXIT_SUCCESS;
  
} /* main */

/*}}}*/

/*}}}*/

/*{{{  Patch for Solaris 1.x */
#if defined(SUN4)
/*
 * On Solaris 1.x the Xmu library appears to be fouled up. This sorts out
 * the mess.
 */

#include <X11/IntrinsicP.h>

/*
 * The following hack is used by XmuConvertStandardSelection to get the
 * following class records.  Without these, a runtime undefined symbol error 
 * occurs.
 */
extern WidgetClass applicationShellWidgetClass,wmShellWidgetClass;

WidgetClass get_applicationShellWidgetClass()
{
    return applicationShellWidgetClass;
}

WidgetClass get_wmShellWidgetClass()
{
    return wmShellWidgetClass;
}
#endif
/*}}}*/
/* end of serverXaw.c */

/*{{{  Emacs Customisation */

/* Local variables: */
/* folded-file: t */
/* end: */

/*}}}*/

