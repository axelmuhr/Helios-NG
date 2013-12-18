
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/refine.c,v 1.3 1993/07/12 11:31:13 nickc Exp $";

/* $Log: refine.c,v $
 * Revision 1.3  1993/07/12  11:31:13  nickc
 * fixed compile time warnings
 *
 * Revision 1.2  1990/08/23  10:28:28  james
 * *** empty log message ***
 *
 * Revision 1.1  90/08/22  16:35:03  james
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
#include <stdlib.h>

static unsigned getbit(char c, string bitchars);

int main(int argc, char **argv)
{
	char dcname[40];
	AccMask setmask = 0;	
	AccMask clearmask = 0;
	char *s;

	if( argc < 2 )
	{
		printf("usage: [[-+=][rwefghvxyzda]] objects...\n");
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
			fprintf(stderr,"could not locate %s : %lx\n",name,Result2(CurrentDir));
			continue;
		}
	
		if( clearmask || setmask )
		{
			AccMask mask = o->Access.Access;
			mask &= ~clearmask;
			mask |= setmask;
			
			e = Refine(o,mask);

			if( e < 0 )
				fprintf(stderr,"refine of %s failed: %lx\n",name,e);
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

