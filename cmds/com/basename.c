/* Written 15/8/90 by paulh for release with HELIOS V1.2 */

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/basename.c,v 1.4 1993/07/12 10:45:08 nickc Exp $";

#include <posix.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void Usage ( void );

char* progname;

int main (
int argc,
char* argv [] )
{
	char *result, *s , *string , *suffix;
	int m , n;
	
	progname = argv[0];
	if (argc < 2)
		Usage ();
	m = strlen ( argv[1] );
	string = (char*) malloc ( m * sizeof (char) + 1);
	strcpy ( string , (char*) argv[1] );
	if ( string[m-1] == '/' ) {
		fprintf ( stdout , "\n" );
		exit (0);
	}
	if (argc == 3) {
		n = strlen ( argv[2] );
		suffix = (char*) malloc ( n * sizeof (char) + 1);
		strcpy ( suffix , (char*) argv[2] );
		if ( m >= n ) {
			if ( strcmp ( &string [m - n] , suffix ) == NULL )
				string [m - n] = '\0';
			else 
				fprintf ( stderr , "%s: %s is not a suffix of %s\n" , progname , suffix , string );
		}
		else	{
			fprintf ( stderr , "%s: Suffix is longer than the string\n" , progname );
			exit (0);
		}
		free ( suffix );
	}
	
	result = (char*) malloc ( 255 * sizeof (char) );
	s = (char*) strtok ( string , "/" );
	while ( s != NULL ) {
		strcpy ( result , s );			
		s = (char*) strtok ( NULL , "/" );
	}
	
	fprintf ( stdout , "%s\n" , result );
	free ( string );
	free ( result );
}

void Usage ()
{
	fprintf ( stderr , "Usage: %s string [suffix]\n" , progname );
	exit (0);
}

