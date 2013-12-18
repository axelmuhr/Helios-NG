#ifndef lint
static char *RCSid = "$Header: efopen.c,v 1.2 87/08/21 16:42:32 rnovak Exp $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /u3/syseng/rnovak/src/lib/RCS/efopen.c,v $
 * $Revision: 1.2 $
 * $Date: 87/08/21 16:42:32 $
 * $State: Exp $
 * $Author: rnovak $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 * $Log:	efopen.c,v $
 * Revision 1.2  87/08/21  16:42:32  rnovak
 * ran this through cb
 * 
 * Revision 1.1  87/08/21  16:32:52  rnovak
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

#ifndef lint
static char rcsid[] = "$Header: efopen.c,v 1.2 87/08/21 16:42:32 rnovak Exp $";
#endif

#include <stdio.h>

FILE *
efopen(file, mode)	/* fopen file, die if cannot */
char *file, *mode;	/* from K & P with addition of perror() and handling
			   of "-" as stdin */
{
	FILE *fp;
	extern char *progname;

	if (strcmp(file, "-") == 0)
		return(stdin);

	if ((fp = fopen(file, mode)) != NULL)
		return (fp);

	if (progname)
		fprintf(stderr, "%s ", progname);
	fprintf(stderr, "can't open file %s mode %s: ", file, mode);
	perror("");
	exit(1);
	/* NOTREACHED */
}
