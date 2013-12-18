static char rcsid[] = "$Header: /usr/perihelion/Helios/filesys/cmds/RCS/access.c,v 1.1 90/10/05 16:40:28 nick Exp $";

/* $Log:	access.c,v $
 * Revision 1.1  90/10/05  16:40:28  nick
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

int main(int argc, char **argv)
{
	char dcname[40];


	if( argc < 2 )
	{
		printf("usage: %s objects...\n",argv[0]);
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
			fprintf(stderr,"could not locate %s : %x\n",name,Result2(CurrentDir));		
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

