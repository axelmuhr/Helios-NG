/***************************************************************************
*
*   rmdir - remove redundant directory.
*	    Directory must be empty before attempting to remove.
*
*   Written by : paulh aka PRH - by judiciously hacking the rm utility 
*   Date       : 21/8/90
****************************************************************************/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/rmdir.c,v 1.3 1993/07/12 12:26:32 nickc Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <gsp.h>
#include <string.h>

int     errors = 0;
int     pflag = 0;
char*	progname;

void usage () {
    fprintf(stderr,"Usage: %s [-p] <directory list>\n", progname);
    exit (1);
}

void del( char *name )
{ 
  if (strcmp ("..", name) && strcmp (".", name))
  {
    word e;
    e = Delete(CurrentDir,name);
    if ( e < 0  ) 
    { fprintf(stderr,"%s: Cannot remove %s: %lx\n", progname, name, e);
      errors++;
    }
  }
}

char* dirname ( char* name )
{
	int index = strlen ( name ) - 1;

	if ((strcmp (name , "/") == 0) || (strcmp (name , "//") == 0)) {
		return ( "/" );
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
	return ( name );
}

void rm (char   *name )
{
	int no_entries;
	word dirsize;
	Stream *s;
	int     isdir;

	Object *o = Locate(CurrentDir,name);

	if( o == Null(Object) )
	{	fprintf(stderr,"Cannot find %s : %lx\n",name, Result2(CurrentDir));
		errors++;
		return;
	}

	isdir = (int)(o->Type & Type_Directory);

    	if (!isdir) 
	{ 	fprintf(stderr,"%s: %s is not a directory\n", progname, name);
		errors++;
		return;
		}
		
	Close (o);
	del(name); 
	
	while ( pflag && ( strcmp ( ( name = dirname (name) ) , "." ) != 0 ) ) {
		o = Locate(CurrentDir,name);
		if( o == Null(Object) )
		{	fprintf(stderr,"Cannot find %s : %lx\n",name, Result2(CurrentDir));
			errors++;
			return;
		}
		
		s = Open(o,NULL,O_ReadOnly);
			
		if( s == Null(Stream) )
		{
			fprintf(stderr,"Cannot open %s : %lx\n",name,Result2(o));
			errors++;
			return;
		}
		
		dirsize = GetFileSize(s);
		
		Close(s);
		Close(o);
					
		no_entries = (int)dirsize/sizeof(DirEntry);
		if (no_entries == 2)
			del (name);
			/* Having only two entries namely . and .. the directory 'name' will be deleted */
			/* without complaint from 'delete'. 						*/
		}	
	return;
}

int main (
	  int     argc,
	  char   *argv[] )
{
  int exstatus;
    char   *opt;

    progname =*argv;
    if (argc < 2)
	usage ();
    argv++;
    --argc;
    while (**argv == '-') {
	opt = *argv;
	while (*++opt != '\0')
	    switch (*opt) {
		case 'p': 
		    pflag++;
		    break;
		default: 
		    fprintf(stderr,"%s: unknown option %c\n", progname, *opt);
		    usage ();
		    break;
	    }
	argc--;
	++argv;
    }
    if (argc < 1) usage ();
    while (argc--) rm (*argv++);
    exstatus = (errors == 0 ? 0 : 1);
    return (exstatus);
}
