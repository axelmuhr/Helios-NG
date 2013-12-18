static char rcsid[] = "$Header: /hsrc/filesys/pfs_v2.1/src/cmds/RCS/refine.c,v 1.1 1992/07/13 16:18:43 craig Exp $";

/* $Log: refine.c,v $
 * Revision 1.1  1992/07/13  16:18:43  craig
 * Initial revision
 *
 * Revision 1.1  90/01/09  13:32:46  chris
 * Initial revision
 * 
 */


#include <stdio.h>
#include <syslib.h>
#include <protect.h>
#include <nonansi.h>

static unsigned getbit(char c, string bitchars);

char *PrgName;

int main(int argc, char **argv)
{
	char dcname[40];
	AccMask setmask = 0;	
	AccMask clearmask = 0;
	char *s;

        PrgName = argv [0];

	if( argc < (1 + 1))
	{
		printf("Usage: %s [[-+=][rwefghvxyzda]] <Objects>\n", PrgName);
		exit(1);
	}

	argv++;
	
	s = *argv;

	if( *s == '+' || *s == '-' || *s == '=' )
	{
		AccMask *mask = &setmask;
		if( *s == '-' ) mask = &clearmask;
		elif( *s == '=' ) clearmask = 0xff;

		s++;
		
		while( *s != '\0' )
		{
			int bit = getbit(*s,FileChars);
			if( bit == 0 ) bit = getbit(*s,DirChars);
			if( bit == 0 ) printf("invalid bit character %c\n",*s);
			*mask |= bit;
			s++;
		}
		argv++;
	}
	
	for( ; *argv != NULL; argv++ )
	{
		char *name = *argv;
		Object *o;
		word e;
		
		o = Locate(CurrentDir,name);
		
		if( o == NULL ) 
		{
			fprintf(stderr,"could not locate %s : %x\n",name,Result2(CurrentDir));		
			continue;
		}
	
		if( clearmask || setmask )
		{
			AccMask mask = o->Access.Access;
			mask &= ~clearmask;
			mask |= setmask;
			
			e = Refine(o,mask);

			if( e < 0 )
				fprintf(stderr,"refine of %s failed: %x\n",name,e);
		}		
		
		DecodeCapability(dcname,&o->Access);	

		printf("%s%s\n",dcname,o->Name);
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

