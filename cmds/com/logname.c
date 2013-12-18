
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/logname.c,v 1.2 1990/08/23 10:19:00 james Exp $";

#include <posix.h>
#include <stdio.h>
#include <stdlib.h>

int main ( argc , argv )

int argc;
char* argv[];

{
	char* logname = malloc ( 255 * sizeof (char));
	char* progname = *argv;

	if ( argc > 1 ) {
		fprintf( stderr , "%s: Parameters ignored\n\n" , progname );
	}
		
	if ( ( logname = getlogin () ) ==  "" ) {
		fprintf( stderr , "%s: Unknown loginame\n" , progname );
		exit (1);
	}
	
	fprintf( stdout , "%s\n" , logname );
	return (0);
}
