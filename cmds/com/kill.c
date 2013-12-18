/*
 * UNIX(tm) like kill command
 *
 * Copyright:	(c) 1991 Perihelion Software Ltd.  All Rights Reserved
 *
 * Author:	N Clifton
 * Date:	$Date: 1991/01/29 09:41:31 $
 * Version:	$Revision: 1.1 $
 */

#include <helios.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <syslib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#define streq(  a, b )		(strcmp(  a, b ) == 0)
#define strneq( a, b, l )	(strncmp( a, b, l ) == 0)


/* global variables */

char *	ProgName =  NULL;		/* name of program as invoked */

struct
{
	char *	abbrev;
	int	value;
} signal_names[] =
{
 { "ABRT",    1 },
 { "FPE",     2 },
 { "ILL",     3 },
 { "INT",     4 },
 { "SEGV",    5 },
 { "TERM",    6 },
 { "STAK",    7 },
 { "ALRM",    8 },
 { "HUP",     9 },
 { "PIPE",    10 },
 { "QUIT",    11 },
 { "TRAP",    12 },
 { "USR1",    13 },
 { "USR2",    14 },
 { "CHLD",    15 },
 { "URG",     16 },
 { "CONT",    17 },
 { "STOP",    18 },
 { "TSTP",    19 },
 { "TTIN",    20 },
 { "TTOU",    21 },
 { "WINCH",   22 },
 { "SIB",     23 },
 { "KILL",    31 }
};

/* functions */

/*
 * error - prints a formatted error message
 *
 * called with a format and parameters ala printf()
 *
 * returns nothing
 *
 * prints out the error message to stderr with the program's name prepended
 * and a terminating new-line.
 * takes care to flush stderr and seek to the end before writing
 */
 
void
error( char * message, ... )
{
	va_list	args;
	

	if (message == NULL || *message == '\0')
		return;
		
	va_start( args, message );

	/* flush error output */
		
	fflush( stderr );
	
	/*
	 * seek to the end, this helps to avoid conflict if the command
	 * is being run in parallel
	 */
	 
	fseek( stderr, 0, SEEK_END );

	/* print the message */
	
	if (ProgName)
		fprintf( stderr, "%s: ", ProgName );
		
	vfprintf( stderr, message, args );
	
	/* add terminating new-line */
	
	fputc( '\n', stderr );
	
	/* flush output again */
	
	fflush( stderr );
	
	va_end( args );
	
	/* finished */
	
	return;
	
} /* error */


/*
 * parse_signal_name - converts a string into a signal number
 *
 * called with NAME which is an ASCII string either for a number
 * of for well known signal abbreviation
 *
 * returns the value of the signal assocaiated with NAME.  If NAME could
 * not be translated returns SIGTERM
 *
 * if NAME is numeric than just takes it value, otherwise look up
 * up NAME in a signal_names table.  Verify the number before returning
 */
 
int
parse_signal_name( char * name )
{
	int	val;
	

	if (name == NULL || *name == '\0')
		return SIGTERM;

	/* check to see if 'name' is numeric */
	
	if ((val = atoi( name )) == 0)
	{
		int	i;
		


		/*
		 * convert 'name' to upper case so that comparisions are
		 * case insensitive
		 */
		 
		for (i = strlen( name ); i--;)
		{
			if (/*isascii( name[ i ] ) &&*/ islower( name[ i ] ))
			{
				name[ i ] = toupper( name[ i ] );
			}
		}

		/*
		 * scan signal name table
		 */
		 
		for (i = sizeof( signal_names ) / sizeof( signal_names[ 0 ] );
		     i--;)
		{
			if (streq( name, signal_names[ i ].abbrev ))
			{
				break;
			}
		}
		
		/*
		 * did we locate a matching abbreviation ?
		 */
		 
		if (i < 0)
		{
			error( "No such signal %s", name );
			
			val = SIGTERM;
		}
		else
		{
			val = signal_names[ i ].value;
		}
	}
	
	/*
	 * ensure that the returned value is within a valid range
	 */
	 
	if (val <= SIGZERO ||
	    val >= NSIG)
	{
		return SIGTERM;
	}
	
	return val;
	
} /* parse_signal_name */


/*
 * send_signal - sends a signal to a named process
 *
 * called with PROCESS_NAME - the name of the process to which the signal should
 *   be sent and SIGNAL - the number of the signal to be sent
 *
 * returns non-zero upon success, zero upon failure
 *
 * examine task name and fill in missing parts if necessary
 * locates the named task, opens it, send the signal, closes it
 */

int
send_signal(
	char *	process_name,
	int	signal )
{
	char		buffer[ 256 ];		/* XXX */
	Object *	object;
	Stream *	stream;
	char *		ptr;
	

	/* locate root */
	
	object = Locate( NULL, "/" );
	
	if (object == NULL)
	{
		return 0;
	}
	
	/* ensure process name points into a likely directory */
	
	if (process_name[ 0 ] == '/')
	{
		strcpy( buffer, process_name );
	}
	else
	{
		strcpy( buffer, "/tasks/" );
		strcat( buffer, process_name );
	}
	
	/* determine if the task name ends in a number */
	
	ptr = strrchr( buffer, '/' );
	ptr = strrchr( ptr, '.' );
	
	if (ptr == NULL)
	{
		int		len;
		DIR	*	tasks;
		struct dirent *	entry;
		
	
		/* find start of task name */
		
		ptr = strrchr( buffer, '/' );
		
		/* open tasks directory */
		
		*ptr = '\0';
		
		tasks = opendir( buffer );
	
		if (!tasks)
		{
			error( "Could not open directory %s", buffer );
			
			*ptr = '/';
		
			Close( object );
			
			return 0;
		}	

		*ptr++ = '/';
		
		len = strlen( ptr );
		
		/* search tasks directory for named process */
		
		while ((entry = readdir( tasks )) != NULL)
		{
			if (strneq( entry->d_name, ptr, len ))
			{
				strcat( buffer, entry->d_name + len );
				
				break;
			}
		}
		
		(void) closedir( tasks );
		
		if (entry == NULL)
		{
			error( "No such task: %s", ptr );
			
			(void) Close( object );
			
			return 1;
		}
	}
	
	/* error( "opening stream %s,and then send signal %d", buffer, signal ); */

	stream = Open( object, buffer, O_WriteOnly );

	if (stream == NULL)
	{
		(void) Close( object );
		
		return 0;
	}
	
	(void) SendSignal( stream, signal );
	
	(void) Close( stream );
	
	return 1;
	
} /* send_signal */
	

/*
 * main - entry point for command
 *
 * called with 1 or more arguments
 * if an argument starts with a dash then it is the
 *      signal number (or ASCII abbreviation) to be sent
 *      to the following arguments, otherwise the
 *      argument names a task to receive a signal.
 *      If no signal is specified then SIGTERM is sent.
 *
 * returns 0 upon success, -1 upon failure (setting errno)
 *
 * parses the arguments and calls send_signal() for each
 * process to be signalled
 */
 
int
main(
	int	argc,
	char **	argv )
{
	int	signal_to_send = SIGKILL;	/* default value */
	int	i;				/* counter */
	char *	arg;				/* argument being processed */
	

	ProgName = argv[ 0 ];
	
	if (argc < 2)
	{
		error( "usage: %s [-signal] task(s)", ProgName );
		
		errno = EINVAL;
		
		return -1;
	}

	for (i = 1; arg = argv[ i ], i < argc; i++)
	{
		if (*arg == '-')
		{
			signal_to_send = parse_signal_name( arg + 1 );
		}
		else
		{
			if (!send_signal( arg, signal_to_send ))
			{
				error( "No Such Process: %s", arg );
				
				errno = ESRCH;
				
				return -1;
			}
		}
	}
	
	return 0;
	
} /* main */
