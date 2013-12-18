/****************************************************************************
*
*   mkdir - make a directory.
*
*   Originally created: NHG
*   re-written as independent program: AE (17th Dec. 1987)
*   Cleaned up a little PAB 20/7/88
* 
****************************************************************************/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/mkdir.c,v 1.3 1991/03/26 12:08:33 martyn Exp $";

#include <syslib.h>
#include <gsp.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	Object *d;
	int i;

	/* ensure that command was invoked with at least one directory
	 * name as an argument */
	if( argc > 1 )
	{
	    /* create each of the directories listed as arguments */
	    for(i=1; i <= (argc-1); ++i)
	    {
                if((d = Locate(CurrentDir, argv[i])) != Null(Object))
		{
		    /* already exists */
		    fprintf(stderr, "mkdir: Cannot create %s - already exists\n",argv[i]);
		    Close(d);
		    return 1;
	 	}

	        d = Create(CurrentDir,argv[i],Type_Directory,0,NULL);
		/* Ensure directory is created */
	        if( d == Null(Object) )
		{
		    /* The directory could not be created */
		    fprintf(stderr, "mkdir: Cannot create %s\n",argv[i]);
		    return 1;
	 	}
		Close(d);
	    }
	}
	else
	{
	    /* No directory names given */
	    fprintf(stderr, "Usage: mkdir dirname ...\n");
	    return 1;
	}

	return 0;
}
/* end of 'mkdir' */
