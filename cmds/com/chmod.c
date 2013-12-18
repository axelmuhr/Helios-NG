
/* $Log: chmod.c,v $
 * Revision 1.3  1993/07/12  11:28:40  nickc
 * fixed compile time warnings
 *
 * Revision 1.2  1990/08/23  10:00:08  james
 * *** empty log message ***
 *
 * Revision 1.1  90/08/22  16:02:21  james
 * Initial revision
 * 
 * Revision 1.1  90/01/09  13:33:01  chris
 * Initial revision
 * 
 */

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/chmod.c,v 1.3 1993/07/12 11:28:40 nickc Exp $";

#include <stdio.h>
#include <syslib.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	Matrix setmask = 0;
	Matrix clearmask = 0;
	
	if( argc == 1 )
	{
		printf("usage: chmod { [vxyz][+-=][rwefghvxyzda] | file }...\n");
		exit( 1 );
	}

	for( argv++; *argv; argv++ )
	{
		char *arg = *argv;
		
		if( (arg[0] == 'v' || arg[0] == 'x' || 
		    arg[0] == 'y' || arg[0] == 'z') && 
		    ( arg[1] == '+' || arg[1] == '-' || arg[1] == '=') )
		{
			int shift;
			Matrix *pmask = &setmask;
			switch( arg[0] )
			{
			default:
			case 'v': shift = 0;  break;
			case 'x': shift = 8;  break;
			case 'y': shift = 16; break;
			case 'z': shift = 24; break;
			}
			if( arg[1] == '-' ) pmask = &clearmask;
			elif( arg[1] == '=' ) 
			{
				clearmask |=  (0xffL << shift);
				setmask   &= ~(0xffL << shift);
			}
			for( arg += 2; *arg; arg++ )
			{
				int bit;
				switch( *arg )
				{
				case 'r': bit = AccMask_R; break;
				case 'w': bit = AccMask_W; break;
				case 'e': bit = AccMask_E; break;
				case 'f': bit = AccMask_F; break;
				case 'g': bit = AccMask_G; break;
				case 'h': bit = AccMask_H; break;
				case 'v': bit = AccMask_V; break;
				case 'x': bit = AccMask_X; break;
				case 'y': bit = AccMask_Y; break;
				case 'z': bit = AccMask_Z; break;
				case 'd': bit = AccMask_D; break;
				case 'a': bit = AccMask_A; break;
				default:
					printf("uexpected character %c - ignored\n",*arg);
					bit = 0;
				}
				*pmask |=  (word) bit << shift;
			}
		}
		else
		{
			ObjInfo info;
			Matrix mat;
			word e;
			
			e = ObjectInfo(CurrentDir,arg,(byte *)&info);
			
			if( e < 0 )
			{
				printf("Cannot find %s : %lx\n",arg,e);
				continue;
			}
			
			mat = info.DirEntry.Matrix;
		
			mat &= ~clearmask;
			mat |= setmask;
			
			e = Protect(CurrentDir,arg,mat);
			
			if( e < 0 )
			{
				printf("Cannot protect %s : %lx\n",arg,e);
				continue;
			}
		}

	} /* argv loop */
}
