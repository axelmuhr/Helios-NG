
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/sleep.c,v 1.4 1993/07/12 11:35:30 nickc Exp $";

#include <stdlib.h>
#include <syslib.h>
#include <stdio.h>
#include <string.h>

void Usage (void);
char* progname;

int main (
int argc,
char* argv[] )
{
	int tm;
	progname = *argv;
	
	if ( argc == 2 ) {
		tm = atoi ( argv [1] );
		if (( tm < 0 ) || ( tm > 2100 )) {
			fprintf (stderr, "%s: time should be a positive integer value less than 2100\n" , progname );
			exit (1);
		}
		Delay ( (word) tm * 1000000 );
	}
	else
	{	
   		Usage ();
	}

	exit (0);
}

void Usage ()

{
        char* name = strrchr(progname,'/');
	fprintf( stderr , "usage: %s time-in-seconds\n" ,
					name ? ++name : progname );
}
