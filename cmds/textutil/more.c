/* 
 *	Author : Andy England  ( - messed around by Tony Cruickshank ).
 *
 *	Currently the program can perform most of the commands inputted
 *	while viewing a file (such as <space>, search, :p, etc).  
 *	The program can also handle the options +linenumber and +/pattern
 * 	used when the program is first called.  Most other such options
 *	are ignored.
 *
 *	Futher hacking by NC 29/11/88
 *
 *	Yet more hacking by PRH alias paulh alias harwoodp 13/9/90 to sort
 *	out keyboard input stream read ops...
 *
 *	Lots of bugs fixed 15/10/90 MJT - more is more BSD-like now. Also
 *      fixed bugs ref 434. Still needs B command adding.
 *
 *	26/03/91
 *	MJT tidied up keyboard stream processing - use stdin for keyboard
 *	input unless data is on a pipe - then use controlling window.
 *	Also fixed Bug #582
 *
 *	crf: 05/06/92
 *	MJT added code to reset attributes of input stream at termination
 *	(05/02/92). However, this only handles the case when you explicilty
 *	quit from more - not the case when more terminates normally.
 *	Have moved the above code from Quit() and made it into a new
 *	function (reset_attr()). This function is now called by Quit()
 *	and also at the end of main().
 *	Note: this fixes Bug 773 which was reported as an ftp bug
 *
 */

#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/textutil/RCS/more.c,v 1.25 1994/03/14 16:14:37 nickc Exp $";
#endif

#include <helios.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <attrib.h>
#include <nonansi.h>
#include <posix.h>
#include <unistd.h>
#include <string.h>

#define CTRL_B 		2
#define CTRL_D 		4
#define BACKSPACE 	8
#define	NEWLINE		'\n'
#define CTRL_L 		12
#define	CARRIAGE_RETURN	'\r'
#define DELETE 		127

#ifndef TRUE
#define FALSE 		0
#define TRUE  		1
#endif

#define END       	0
#define MORE      	1
#define NOPROMPT  	2
#define PROMPT    	3

#define ERROR        	1
#define SPACE        	2
#define ONELINE      	3
#define SCROLL       	4
#define SPACERESET   	5
#define SKIPLINES    	6
#define SKIPSCREENS  	7
#define QUIT         	8
#define LINENUM      	9
#define ED           	10
#define HELP         	11
#define SEARCH       	12
#define TAKEBACK     	13
#define PREVFILE     	14
#define NEXTFILE     	15
#define DISPCURRENT	16
#define BACK		17
#define REFRESH		18

#define	DISPLAY		1
#define NODISPLAY	0

#define MAXSTR		256

FILE * 	Debug;
FILE * 	File;
FILE * 	InputStr;
int     Column      	= 0;
int     Screenful   	= 23;
int	WindowSize  	= 23;
int     ScrollSize  	= 11;
int     ScreenWidth 	= 80;
char    Expr[ MAXSTR ];
char    PrevExpr[ MAXSTR ];
char    Comm[ MAXSTR ];
char    PrevComm[ MAXSTR ];
int     FileSize;

char*	progname;
char*	editor;

int 	HeaderFlg 	= FALSE;
int	HeaderPrntd 	= FALSE;
int	NoAdd 		= FALSE;
int 	PipePresent 	= FALSE;
int	WriteInfo 	= PROMPT;
int	DisplayError 	= FALSE;
int	splitline	= FALSE;
int	OutputToScreen	= FALSE;

int 	FileNumber 	= 0;
int	LineNumber 	= 0;
int	LastLine 	= 0;
long	Position 	= 0;
long	LastPos 	= 0;
long	SearchStrtP 	= 0;
int 	SearchStrtLn 	= 0;
char **	FileList;
char **	FilePntr;
int	Command;
int	LastComm;
int	Count;
int	CountPresent;

int	FormFeed 	= FALSE;
int	Underlining 	= FALSE;
int	Page 		= FALSE;
int	Squeeze 	= FALSE;
int	NoFold 		= FALSE;
int	ExprStrt 	= FALSE;
int	LineStrt 	= FALSE;

int	Search( int n );
int	Interpret( char c );
int	Process_Data(int n , int dflag);
void	Clear(void);
char   *Command_Parse(char *p);
char   *nstrcpy(char *to, char *from, char *start);
#ifdef __HELIOS
void reset_attr(void) ;
#endif

void
Usage( void )
{
	fprintf( stderr, "Usage: %s [-cdfls] [-n] [+linenum | +/pattern] [name ...]\n" , progname );
}

void
Wipeout( void )
{
	if(OutputToScreen)
		{
		fprintf( stdout, "\r                                                                               \r" );
		fflush( stdout );
		}
}

void
Quit( void )
{
    Wipeout();
    fclose( File);

#ifdef __HELIOS
    reset_attr() ;
#endif
	
    exit( 0 );
}

void	
onintr( void )
{
	Quit();
}

void
Initialize(
	int     	argc,
	char **		argv )
{
	char *		p;
	Stream *	output = fdstream( fileno( stdout ) );
	Attributes	attr;

	progname = *argv;
	
	if (( editor = getenv ( "EDITOR" )) == NULL ) {		/* EDITOR defines the system text editor */
		editor = (char*) malloc ( 6 * sizeof ( char ));
		strcpy ( editor , "emacs" );
	}

	if(isatty(fileno(stdout)))
		{
			
		GetAttributes( output, &attr );
	
		ScreenWidth = attr.Time;
		Screenful   = attr.Min - 2;
		WindowSize  = Screenful;
		}

	if (argc == 1 && isatty( fileno( stdin ) ))
	{
		Usage();
		exit( 1 );
	}

	if (isatty( fileno( stdout ) ))
		OutputToScreen = TRUE;

#if 0
	while ( **(++argv) == '-' || **argv == '+' )
#else
/*
-- crf: 08/09/92 - protect against NULL pointer access
*/
	while (( *(++argv) != NULL ) &&
	      ( **argv == '-' || **argv == '+' ))
#endif
	{
		--argc;
		p = *argv;

		if (*p == '-')
		{
			++p;

			if (isdigit( *p ))
			{
				WindowSize = 0;
				do
				{
					WindowSize = WindowSize * 10 + *p -'0';
					++p;
				}
				while (isdigit( *p ));
			}
			else
			{
				while (*p)
				{
					switch (*p)
					{
					case 'c' :
						Page = TRUE;
						WindowSize++;
						break;
					case 'd' :
						DisplayError = TRUE;
						break;
					case 'f' :
						NoFold = TRUE;
						break;
					case 'l' :
						FormFeed = TRUE;
						break;
					case 's' :
						Squeeze = TRUE;
						break;
					case 'u' :
						Underlining = TRUE;
						break;
					}

					++p;
				}
			}
		}
		else
		{
			if (*(++p) == '/')
			{
				ExprStrt     = TRUE;
				LineStrt     = FALSE;
				Count        = 1;
				CountPresent = FALSE;

				strcpy( Expr, ++p );
			}
			else
			{
				ExprStrt     = FALSE;
				LineStrt     = TRUE;
				Count        = 0;
				CountPresent = TRUE;

				while (isdigit( *p ))
				{
					Count = Count * 10 + (*p - '0');
					++p;
				}

				--Count; /* start at Count not skip Count */
					 /* many lines                    */
			}
		}
	}

	FileNumber = argc - 1;
	FileList   = argv;

#ifdef __HELIOS
	if (GetAttributes( fdstream( fileno( InputStr ) ), &attr ) < 0)
	{
		exit( 4 );
	}
	
	AddAttribute(    &attr, ConsoleRawInput );
	RemoveAttribute( &attr, ConsoleEcho );
		
	if (SetAttributes( fdstream( fileno( InputStr ) ), &attr ) < 0)
	{
		exit( 5 );
	}
#else
	system( "stty -echo" );
	system( "stty raw" );
#endif

	signal( SIGINT, (VoidFnPtr) onintr );
}

int
FSkipBack( void )
{
	Wipeout();

	fprintf( stdout, "\n...skipping\n\r" );
		
	Count = CountPresent ? Count : 1;
	
	if (NoAdd)
		FilePntr--;
	
	while ( Count-- )
		if (FilePntr != FileList)
			FilePntr--;

	fprintf( stdout, "...skipping back to %s\n\n\r", *FilePntr );

	return END;
}

int
Invoke( char * command )
{

	Wipeout();
	fprintf( stdout, "%s\r", command );
	fflush(  stdout );
	system(  command );
	fprintf( stdout, "------------------------\n\r" );
	fflush(  stdout );

	return MORE;
}

int
GetLine(
	char    Prompt,
	char *	Buffer )
{
	int 	count = 0;
	int 	c;


	Wipeout();

	putchar( Prompt );
	fflush( stdout );

	while ((c = fgetc( InputStr )) != CARRIAGE_RETURN && c != NEWLINE)
	{
		if (isprint( c ))
		{
			putchar( c );
			fflush( stdout );
			Buffer[ count++ ] = c;
		}
		else
		{
			if (c == DELETE || c == BACKSPACE)
			{
				putchar( '\b' );
				putchar( ' ' );
				putchar( '\b' );
				fflush( stdout );

				if (count == 0)
				{
					Buffer[ 0 ] = '\0';
					return ( 0 );
				}

				count--;
			}
		}
	}

	putchar( CARRIAGE_RETURN );
	fflush( stdout );

	Buffer[ count ] = '\0';

	return 1;
}

void
TakeBack( void )
{
	Wipeout();

	fprintf( stdout, "\n\r*** Back ***\n\r\n\r" );

	fseek( File, SearchStrtP, 0 );

	Position   = SearchStrtP;
	LineNumber = SearchStrtLn;
}

void
Refresh( void )
{
	Clear();

	fseek( File, LastPos, 0 );

	Position   = LastPos;
	LineNumber = LastLine;
}


/* Skip forward n many lines */

int
SkipForward( int n )
{
	if (WriteInfo != NOPROMPT && OutputToScreen)
	{
		Wipeout();
		fprintf( stdout, "\n\r...skipping %d lines\n\r\n\r", n );
	}

	return Process_Data(n, NODISPLAY);
}

int
Help( void )
{
	Wipeout();

	if(Page)
		Clear();

	fprintf( stdout, "Most commands optionally preceded by integer argument.  Defaults in brackets.   \n" );
	fprintf( stdout, "Star (*) indicates argument becomes new default.                                \n" );
	fprintf( stdout, "--------------------------------------------------------------------------------\n" );
	fprintf( stdout, "\n");
	fprintf( stdout, "<space>                           - next page                                   \n" );
	fprintf( stdout, "z                                 - set scrolling window size to (1) *          \n" );
	fprintf( stdout, "<return>                          - next line                        *          \n" );
	fprintf( stdout, "d or ctrl-D                       - scroll forwards (11) line(s)     *          \n" );
	fprintf( stdout, "q or Q or <interrupt>             - quit                                        \n" );
	fprintf( stdout, "s                                 - skip forward (1) line(s)                    \n" );
	fprintf( stdout, "f                                 - skip forward (1) screenful(s)               \n" );
	fprintf( stdout, "'                                 - skip to start of previous search            \n" );
	fprintf( stdout, "/<regular expression>             - search forward                              \n" );
	fprintf( stdout, "n                                 - repeat last search                          \n" );
	fprintf( stdout, "h                                 - this message                                \n" );
	fprintf( stdout, "!<cmd> or :!<cmd>                 - shell escape                                \n" );
	fprintf( stdout, "v                                 - invoke editor on the file                   \n" );
	fprintf( stdout, "=                                 - report current line number                  \n" );
	fprintf( stdout, ":n                                - skip forwards (1) file                      \n" );
	fprintf( stdout, ":p                                - skip backwards (1) file                     \n" );
	fprintf( stdout, ":f                                - report current filename and linenumber      \n" );
	fprintf( stdout, ".                                 - repeat last command                         \n" );
	fprintf( stdout, "ctrl-L                            - redraw screen                               \n" );
	fprintf( stdout, "--------------------------------------------------------------------------------\n" );

	return MORE;
}

void
PrintHdr( void )
{
	if (FileNumber > 1 && !HeaderFlg &&
	    Command != PREVFILE && Command != NEXTFILE && Command != ED)
	{
		Wipeout();

		if(Page)
			Clear();
		fprintf( stdout, "::::::::::::::\n\r" );
		fprintf( stdout, "%s\n\r", *FilePntr );
		fprintf( stdout, "::::::::::::::\n\r" );

		HeaderFlg   = TRUE;
		HeaderPrntd = TRUE;
	}
	else
		HeaderPrntd = FALSE;
}

void
Start( void ) 
{
	if (ExprStrt)
	{
		Command   = SEARCH;
		WriteInfo = NOPROMPT;
		ExprStrt  = FALSE;
		return;
	}
	else if (LineStrt)
	{
		Command   = SKIPLINES;
		WriteInfo = NOPROMPT;
		LineStrt  = FALSE;
		return;
	}
	else
		Command   = SPACE;
}

void
Reset( void )
{
	LastLine = LineNumber = SearchStrtLn = 0;
	LastPos = Position   = SearchStrtP  = 0;
	Column     = 0;
	splitline = FALSE;
}

void
GetComm( void )
{
	int c;

	/* Check for piped stdout */

	if (!isatty( fileno( stdout )))
	{
		Command = SPACE;
		return;
	}

	Count        = 0;
	CountPresent = FALSE;
	
	c = fgetc( InputStr );

	/* catch end of file - this can happen if someone types ctrl-C */
	
	if (c == EOF)
	  {
	    Command = QUIT;
	    return;
	  }
	
	if (isdigit( c ))
	{
		CountPresent = TRUE;

		do
		{
			Count = Count * 10 + (c - '0');
			c     = fgetc( InputStr );
		}
		while (isdigit ( c ));
	}

	unless (Count)
		Count = 1;

	Command = Interpret( tolower (c) );
}

int
Display( int n , int clearscreen)
{
	LastPos = Position;
	LastLine = LineNumber;

	if ( n >= Screenful || clearscreen)
	{
		if(Page && !HeaderPrntd)
			Clear();
		else
			Wipeout();
	}
	else
	{
		Wipeout();
	}

	return Process_Data(n, DISPLAY);
}

int
CarryOut( void )
{
	int 	scr; 
	char 	comm[ MAXSTR ];
	
 
	if (!PipePresent)
		PrintHdr();

	switch (Command)
	{
	default: 
		return MORE;

	case SPACE :
		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		if (HeaderPrntd && Screenful - 2 < WindowSize)
			scr = (Page ? Screenful - 2 : Screenful - 3 ) ;
		else
			scr = WindowSize ;

		return (Display( CountPresent ? Count : scr , TRUE));

	case ONELINE :
		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		if (HeaderPrntd && Screenful - 2 < Count)
			scr = Screenful - 2;
		else
			scr = Count;

		if (CountPresent)
			ScrollSize = Count;

		return (Display( scr , FALSE));

	case SCROLL :
		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		if (CountPresent)
			ScrollSize = Count;

		return (Display( ScrollSize , FALSE));

	case SPACERESET :
		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		if (CountPresent)
			WindowSize = Count;

		return (Display( WindowSize , TRUE));

	case SKIPLINES :
		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		if (SkipForward( Count ) == END)
			return END;
		else
			return (Display( WindowSize - 1 , TRUE));

	case SKIPSCREENS :
		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		if (SkipForward( Count * WindowSize ) == END)
			return END;
		else
			return (Display( WindowSize - 1 , TRUE));

	case QUIT :
		fclose( File );

		Quit();

		return END;

	case LINENUM :
		Wipeout();

		fprintf( stdout, "%d\r", LineNumber );

		fflush( stdout );

		return NOPROMPT;

	case ED : 
		if (PipePresent)
			return ERROR;
		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		if (strcmp(editor,"emacs") == 0)
			sprintf( comm, "emacs %s", *(--FilePntr) );
		else
		if (strcmp(editor,"vi") == 0)
			sprintf( comm, "vi +%d %s", LineNumber, *(--FilePntr) );
		else
			sprintf( comm, "%s %s", editor, *(--FilePntr) );

		Wipeout();

		Invoke( comm );

		FilePntr++;

		return MORE;

	case HELP :
		return (Help());

	case SEARCH :
		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		splitline = FALSE;
		
		return (Search( Count ));

	case TAKEBACK :
		if (PipePresent)
			return ERROR;

		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		TakeBack();

		splitline = FALSE;

		return (Display( WindowSize , TRUE));

	case REFRESH :
		if (PipePresent)
			return ERROR;

		if (!NoAdd)
		{
			++FilePntr;
			NoAdd = TRUE;
		}

		Refresh();

		return (Display( WindowSize , TRUE));

	case PREVFILE :
		if (PipePresent)
			return ERROR;

		splitline = FALSE;
		return (FSkipBack());

	case NEXTFILE :
		Wipeout();

		if (!PipePresent)
		{
			if (!NoAdd) ++FilePntr;

			while ( CountPresent && (*FilePntr != NULL) && --Count )
				FilePntr++;
				
			if (*FilePntr == NULL)
				FilePntr--;
				
			fprintf( stdout, "\n...skipping\n\r" );
			fprintf( stdout, "...skipping to %s\n\n\r", *FilePntr );
		}

		splitline = FALSE;

		return END;
		
	case DISPCURRENT :
		Wipeout();
		
		fprintf( stdout, "\"%s\" line %d\r" , (FilePntr != FileList ) ? *(FilePntr-1) : *FilePntr , LineNumber );
		
		fflush( stdout );
		
		return NOPROMPT;
		
	}
}

int
Interpret( char c )
{
	int retval = ERROR;


	switch (c)
	{
	default: 
		break;

	case ' ': 
		retval = SPACE;
		break;

	case CARRIAGE_RETURN: 
	case NEWLINE: 
		retval = ONELINE;
		break;

	case CTRL_D: 
	case 'd': 
		retval = SCROLL;
		break;

	case 'z': 
		retval = SPACERESET;
		break;

	case 's': 
		retval = SKIPLINES;
		break;

	case 'f': 
		retval = SKIPSCREENS;
		break;

	case 'q': 
		Quit();

	case '=': 
		retval = LINENUM;
		break;

	case 'v': 
		retval = ED;
		break;

	case 'h': 
		retval = HELP;
		break;

	case '/': 
		nstrcpy ( PrevExpr, Expr , PrevExpr);
		if (GetLine( '/', Expr ))
			{
			if(Expr[0] == '\0')
				nstrcpy ( Expr, PrevExpr , Expr);
			retval = SEARCH;
			}
		else
			{
			nstrcpy ( Expr, PrevExpr , Expr);
			retval = ERROR;
			}
		break;

	case 'n': 
		retval = SEARCH;
		break;

	case '\'': 
		retval = TAKEBACK;
		break;

	case 'b': 
	case CTRL_B:
		retval = BACK;
		break;

	case CTRL_L:
		retval = REFRESH;
		break;

	case '!': 
		nstrcpy ( PrevComm, Comm , PrevComm);
		if (GetLine( '!', Comm ))
			Invoke( Command_Parse ( Comm ) );
		else
			nstrcpy ( Comm, PrevComm , Comm);
		break;

	case ':': 
		switch (tolower(fgetc( InputStr )))
		{
		case 'p' :
			retval = PREVFILE;
			break;
		case 'n' :
			retval = NEXTFILE;
			break;
		case 'f' :
			retval = DISPCURRENT;
			break;
		case '!':
			nstrcpy ( PrevComm, Comm , PrevComm);
			if (GetLine( '!', Comm ))
				Invoke( Command_Parse ( Comm ) );
			else
				nstrcpy ( Comm, PrevComm , Comm);
			break;
		case 'q' :
			Quit();
		default :
			retval = ERROR;
			break;
		}
		break;

	case '.' :
		retval = LastComm;
		break;

	}

	if (retval != ERROR && c != '.')
		LastComm = retval;

	return (retval);
}

int
Search( int n )
{
	int 	c;
	char	buffer[ MAXSTR ];
	int 	expr_pntr = 0,
		buff_pntr = 0;
	long 	buf_start = 0;
	long	npos;
	int	nline;

	buf_start = npos = Position;
	nline = LineNumber;

	if(Expr[0] == '\0' && OutputToScreen)
		{
		fprintf( stdout, "\rNo previous search pattern\r");
		fflush ( stdout );
		return NOPROMPT;
		}
	
	if(OutputToScreen)
		{
		Wipeout();

		fprintf( stdout, "/%s\r", Expr);
		fflush(stdout);
		}

	buffer[0] = '\0';

	while( ( c = fgetc( File ) ) != EOF )
	{
		npos++;

		buffer[ buff_pntr ] = c;

		++buff_pntr;

		buffer[ buff_pntr ] = '\0';

		if ( Expr[ expr_pntr ] == c )
			++expr_pntr;
		else
		        expr_pntr = 0;

		if ( Expr[ expr_pntr ] == '\0' )
		{
		    --n;
		    expr_pntr = 0;
		}

		if (c == NEWLINE)
		{
			nline++;

			buf_start = npos;

			buff_pntr = 0;

			buffer[ 0 ] = '\0';
		}

		if ( n == 0 )
		{
			SearchStrtLn = LineNumber;
			SearchStrtP  = Position;

			if(OutputToScreen && (nline -LineNumber) > 1)
				{
				fprintf( stdout, "\n\r...skipping %d lines\n\r\n\r", nline - LineNumber );
				fflush( stdout );
				}

			if (PipePresent)
			{
				/* Fill buffer - */
				while ( (c = fgetc ( File )) != NEWLINE && c != EOF)
				{
					buffer[ buff_pntr ] = c;

					++buff_pntr;

					buffer[ buff_pntr ] = '\0';
				}

				fprintf( stdout, "%s\n\r", buffer );
			}

			else
			{
				fseek( File, buf_start, 0 );
				Position = buf_start;
			}

			LineNumber = nline;

			return( Display( WindowSize - 1 , TRUE));
		}
	}

	if (PipePresent)
	{
		if(OutputToScreen)
			fprintf( stdout, "\rPattern not found\r\n" );
		return END;
	}
	else
	{
		fseek( File, Position, 0 );	 /* rewind to start */

		if(OutputToScreen)
			{
			Wipeout();

			fprintf( stdout, "\rPattern not found\r" );

			fflush( stdout );
			}

		return NOPROMPT;
	}
}

void
More( void )
{
	int res;
	do
	{
		res = CarryOut();

		WriteInfo = PROMPT;

		if (res == MORE && OutputToScreen)
		{
			Wipeout();

			fprintf( stdout, "--More--" );

			fflush( stdout );

			if (!PipePresent && FileSize)
			{
				fprintf( stdout, "(%ld%%)", (Position * 100) / FileSize );
				fflush(  stdout );
			}

			if (DisplayError)
			{
				fprintf( stdout, "[Press space to continue, 'q' to quit.]");
				fflush( stdout );
			}
		}

		if (res != END)
			GetComm();
	}
	while (res != END);

	HeaderFlg   = FALSE;
	HeaderPrntd = FALSE;
	NoAdd       = FALSE;

	fclose ( File );
}

int
main(
	int 	argc,
	char **	argv )
{
	struct stat 	stbuf;
	char *instr;
	char *ctermid ( char* );

	if(isatty(fileno(stdin)))
		InputStr = stdin;
	else
		{
		instr = (char *) (malloc ( L_ctermid * sizeof (char)));
		InputStr = fopen ( ctermid ( instr ) , "r" );
		}

	setvbuf(InputStr, NULL, _IONBF, 0);
	
	Initialize ( argc , argv );

	Start ();

	/* check for pipe */

	if (!isatty( fileno( stdin ) ))
	{

		File        = stdin;
		PipePresent = TRUE;

		Reset();
		More();
	}

	PipePresent = FALSE;

	for (FilePntr = FileList; *FilePntr; )
	{
		if (stat( *FilePntr, &stbuf ))
		{
			fprintf( stderr, "%s: No such file or directory\n\r", *FilePntr );
			++FilePntr;
		}
		else if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
		{
			fprintf( stderr, "\n\r*** %s: directory ***\n\r\n\r", *FilePntr );
			++FilePntr;
		}
		else if ((File = fopen( *FilePntr, "rb" )) == NULL)
		{
			fprintf( stderr, "Can't open %s\n\r", *FilePntr );
			++FilePntr;
		}
		else
		{
			FileSize = (int) stbuf.st_size;

			Reset();
			More();

			if (*FilePntr != NULL)
			{
				if(OutputToScreen)
					{
					Wipeout();

					fprintf( stdout, "--More--(Next file: %s)\r", *FilePntr );
					fflush(  stdout);
					}

				GetComm();
			}
		}
	}
#ifdef __HELIOS
	reset_attr() ;
#endif
}


int
Process_Data(int n , int dflag)
{
	int 	c, lc, pc;
	int	gotformfeed = FALSE;

	pc = '\0';
	lc = NEWLINE;

	while ((c = fgetc( File )) != EOF)
	{
		Position++;

		if (c == CARRIAGE_RETURN)
			continue;
		if (c == NEWLINE && splitline)
			{
			splitline = FALSE;
			continue;
			}

		splitline = FALSE;

		if (c == '\f' && !FormFeed && (Position != 1))
			{
			if(dflag)
				{
				putchar( '^' );
				putchar( 'L' );
				}
			Column++;
			gotformfeed = TRUE;
			}

		else
			{
			unless (c == NEWLINE)
				{
				if(Squeeze && pc == NEWLINE && lc == NEWLINE && dflag)
					{
					putchar(NEWLINE);
					putchar(CARRIAGE_RETURN);
					}
				if(dflag && !(c == '\t' && Column > ScreenWidth - 8))
					putchar( c );
				}
			}

		if (c == '\t')
			while((++Column) % 8);

		if (c != NEWLINE)
			{
			if (++Column >= ScreenWidth)
				unless (NoFold)
					{
					c = NEWLINE;
					splitline = TRUE;
					}
			}

		if (c == NEWLINE)
			{

			LineNumber++;

			if(Squeeze)
				{
				if(lc != NEWLINE && dflag)
					{
					putchar ( c );
					putchar (CARRIAGE_RETURN);
					}
				}
			else
				if(dflag)
					{
					putchar(c);
					putchar (CARRIAGE_RETURN);
					}

			Column = 0;

			if ((--n == 0) || gotformfeed)
				{
				if(dflag)
					fflush( stdout );
				return MORE;
				}
			}

		pc = lc;
		lc = c;
		}

	if(dflag)
		fflush( stdout );
	return END;
}

void
Clear(void)
{
	if(OutputToScreen)
		{
		putchar('\f');
		fflush(stdout);
		}
}

char *
Command_Parse(char *p)
{
	char tmpbuffer[MAXSTR], *tp, c;

	tp = tmpbuffer;

	while ((c = *p++) != '\0')
		switch(c)
			{
			case '\\':
				*tp++ = *p++;
				continue;

			case '%':
				if(FilePntr != FileList)
					{
					nstrcpy( tp, *(FilePntr-1), tmpbuffer);
					tp += strlen( *(FilePntr-1));
					}
				else
					{
					nstrcpy( tp, *FilePntr, tmpbuffer);
					tp += strlen( *FilePntr);
					}
				continue;

			case '!':
				nstrcpy(tp, PrevComm, tmpbuffer);
				tp += strlen (PrevComm);
				continue;

			default:
				*tp++ = c;
			}
	*tp = 0;
	return nstrcpy(Comm, tmpbuffer, Comm);
}

char *
nstrcpy( char *to, char *from, char *start)
{
	while ((*to++ = *from++) != '\0')
		if (to - start >= MAXSTR - 1)
			break;
	*to = 0;

	return(start);
}

#ifdef __HELIOS
void reset_attr()
{
	Attributes attr;

	if (GetAttributes( fdstream( fileno( InputStr ) ), &attr ) < 0)
	{
		exit( 4 );
	}
	
	RemoveAttribute( &attr, ConsoleRawInput );
	AddAttribute( &attr, ConsoleEcho );
		
	if (SetAttributes( fdstream( fileno( InputStr ) ), &attr ) < 0)
	{
		exit( 5 );
	}
}
#endif
