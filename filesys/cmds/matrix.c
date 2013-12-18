static char rcsid[] = "$Header: /usr/perihelion/Helios/filesys/cmds/RCS/matrix.c,v 1.1 90/10/05 16:40:38 nick Exp $";

/* $Log:	matrix.c,v $
 * Revision 1.1  90/10/05  16:40:38  nick
 * Initial revision
 * 
 * Revision 1.1  90/01/09  13:32:51  chris
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
		char *s;
		ObjInfo info;
		char *type;
		char tt[20];
				
		if( ObjectInfo(CurrentDir,name,(byte *)&info) != 0 ) 
		{
			fprintf(stderr,"could not locate %s : %x\n",name,Result2(CurrentDir));		
			continue;
		}
	
		switch( info.DirEntry.Type )
		{
		case Type_Directory: type = "d"; break;
		case Type_File: type = "f"; break;
		case Type_Link: type = "l"; break;
		default:
			type = tt;
			sprintf(tt,"%x",info.DirEntry.Type);
			break;	
		}
		DecodeMatrix(dcname,info.DirEntry.Matrix,info.DirEntry.Type);
		printf("%s %s %s\n",type,dcname,info.DirEntry.Name);
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

