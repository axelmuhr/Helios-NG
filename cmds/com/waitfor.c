
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/waitfor.c,v 1.3 1993/07/12 11:48:40 nickc Exp $";

#include <syslib.h>
#include <stdlib.h>
#include <stdio.h>

char* progname;

void Usage ( void )
{
	fprintf( stderr , "Usage: %s servername\n" , progname );
	exit (0);
}

int main (
int argc,
char* argv [] )
{
	Object*	obj;
	

	progname = *argv;
	if ( argc != 2 )
		Usage ();
		
	while ( ( obj = Locate ( NULL , argv[1] ) ) == NULL )
		Delay(OneSec);
		
	return (0);
}

