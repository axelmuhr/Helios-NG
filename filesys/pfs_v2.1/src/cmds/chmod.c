static char rcsid[] = "$Header: /hsrc/filesys/pfs_v2.1/src/cmds/RCS/chmod.c,v 1.1 1992/07/13 16:18:43 craig Exp $";

/* $Log: chmod.c,v $
 * Revision 1.1  1992/07/13  16:18:43  craig
 * Initial revision
 *
 * Revision 1.1  90/01/09  13:33:01  chris
 * Initial revision
 * 
 */

#include <stdio.h>
#include <syslib.h>
char *PrgName;

int main(int argc, char **argv)
{
	Matrix setmask = 0;
	Matrix clearmask = 0;
	
	PrgName = argv [0];

	if( argc == (1 + 0))
	{
		fprintf(stderr, "Usage: %s { [vxyz][+-=][rwefghvxyzda] | file }...\n", PrgName);
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
			case 'v': shift = 0;  break;
			case 'x': shift = 8;  break;
			case 'y': shift = 16; break;
			case 'z': shift = 24; break;
			}
			if( arg[1] == '-' ) pmask = &clearmask;
			elif( arg[1] == '=' ) 
			{
				clearmask |= (0xff<<shift);
				setmask &= ~(0xff<<shift);
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
					fprintf(stderr, "%s: unexpected character %c - ignored\n", PrgName, *arg);
					bit = 0;
				}
				*pmask |= bit<<shift;
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
				fprintf(stderr, "%s: Cannot find %s : %x\n",PrgName, arg,e);
				continue;
			}
			
			mat = info.DirEntry.Matrix;
		
			mat &= ~clearmask;
			mat |= setmask;
			
			e = Protect(CurrentDir,arg,mat);
			
			if( e < 0 )
			{
				fprintf(stderr, "%s: Cannot protect %s : %x\n", PrgName, arg,e);
				continue;
			}
		}

	} /* argv loop */
}
