/**************************************************************************
*
*   ln - link files.
*
*   originally created: NHG
*   re-written as independent program: AE (17th Dec. 1987)
*   Slightly munged by PAB - ignore -s +
*
**************************************************************************/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/ln.c,v 1.3 1993/07/12 11:48:49 nickc Exp $";

#include <stdio.h>
#include <syslib.h>
#include <string.h>

extern	void exit(int);

int
main(int argc,char **argv)
{
    Object *o;
    word e;

	/* check that caller supplied enough arguments */
	if( argc != 3 )
	{
		if ( argc == 4 && strcmp(argv[1],"-s") == 0)
			argv++;
		else
		{
	        	fprintf(stderr,"Usage: ln <source> <dest>\n");
		        exit(1);
		}
	}

	o = Locate(CurrentDir,argv[1]);
	
	if( o == Null(Object) )
	{
		fprintf(stderr, "ln: Cannot locate %s - %lx\n",argv[1],Result2(CurrentDir));
		exit(1);
	}

	e = Link(CurrentDir,argv[2],o);
	if( e < 0 )
	{	fprintf(stderr,"ln: Link failed - %lx\n",e);
		Close(o);
		exit(1);
	}

	Close(o);

return 0;
}

/* end of 'ln' */


