/***************************************************************************
*
*  dirname - remove filename element from a pathname.
*
*   Written by : paulh aka PRH
*   Date       : 21/8/90
*
*   I am not completely happy with the way this routine works - it does
*   not match the specification in POSIX precisely. 
*   dirname is used almost line for line in rmdir, so any improvements should
*   be replicated in rmdir also.
*
****************************************************************************/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/dirname.c,v 1.2 1990/08/23 10:08:49 james Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dirname ( char* );
void Usage ( void );

char* progname;

int main ( int argc , char* argv[] )
{
	progname = argv[0];
	if (argc != 2 )
		Usage ();
	dirname ( argv [1] );
	return 0;
}

void dirname ( char* name )
{
	int index = strlen ( name ) - 1;

	if ((strcmp (name , "/") == 0) || (strcmp (name , "//") == 0)) {
		fprintf ( stdout , "/\n" );
		return;
	}
	
	while ( index && (name[index] == '/') ) {	/* Ommit trailing / */
		name[index] = '\0';
		index--;
	}
		
	while ( ( name[index] != '/' ) && ( index > 0 ) )
		index--;

	if (!index)
	{
		strcpy ( name , "." );
		strcat ( name , "\0" );
	}
	else
	{
		name[index] = '\0';
	}
	fprintf ( stdout, "%s\n", name );
}

void Usage ()
{
	fprintf ( stderr , "Usage: %s pathname\n" , progname );
	exit (0);
}

