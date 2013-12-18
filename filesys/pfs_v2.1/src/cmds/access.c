static char rcsid[] = "$Header: /hsrc/filesys/pfs_v2.1/src/cmds/RCS/access.c,v 1.1 1992/07/13 16:18:43 craig Exp $";

/* $Log: access.c,v $
 * Revision 1.1  1992/07/13  16:18:43  craig
 * Initial revision
 *
 * Revision 1.1  90/01/09  13:48:37  chris
 * Initial revision
 * 
 */

#include <stdio.h>
#include <syslib.h>
#include <protect.h>
#include <nonansi.h>
#include <gsp.h>

static unsigned getbit(char c, string bitchars);

char *PrgName;

int main(int argc, char **argv)
{
	char dcname[40];
	
	PrgName = argv [0];
	
	if( argc < (1 + 1))
	{
		fprintf(stderr, "Usage: %s <Objects>\n", PrgName);
		exit(1);
	}

	argv++;
	
	for( ; *argv != NULL; argv++ )
	{
		char *name = *argv;
		Object *o;
		char *s;
		char *chars;
		char *type;
		char tt[20];
		
		o = Locate(CurrentDir,name);
		
		if( o == NULL ) 
		{
			fprintf(stderr,"%s: Could not locate %s : %x\n",PrgName, name,Result2(CurrentDir));		
			continue;
		}
	
		dcname[0] = 0;
		switch( o->Type )
		{
		case Type_Directory: type = "d"; break;
		case Type_File: type = "f"; break;
		default:
			type = tt;
			sprintf(tt,"%x",o->Type);
			break;	
		}
		chars = getbitchars(o->Type);
		s = DecodeMask(dcname,o->Access.Access,chars);
		*s = 0;
		printf("%s %s %s\n",type,dcname,o->Name);
	}

	return 0;
}

static unsigned getbit(char c, string bitchars)
{
	unsigned bit = 0;
	while( *bitchars )
		if( c == *bitchars ) return 1<<bit;
		else bitchars++,bit++;
	return 0;
}

