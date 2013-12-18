#ifndef lint
static char *RCSid = "$Header: strcar.c,v 1.1 87/08/21 16:34:11 rnovak Exp $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /u3/syseng/rnovak/src/lib/RCS/strcar.c,v $
 * $Revision: 1.1 $
 * $Date: 87/08/21 16:34:11 $
 * $State: Exp $
 * $Author: rnovak $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 * $Log:	strcar.c,v $
 * Revision 1.1  87/08/21  16:34:11  rnovak
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

/*
 *	$Header: strcar.c,v 1.1 87/08/21 16:34:11 rnovak Exp $
 *	$Log:	strcar.c,v $
 * Revision 1.1  87/08/21  16:34:11  rnovak
 * Initial revision
 * 
 * Revision 1.2  86/12/29  16:10:07  rnovak
 * Handle the includes for an include directory.
 * 
 * 
 * Revision 1.1  86/12/29  15:23:51  rnovak
 * Initial revision
 * 
 * Revision 1.3  86/12/29  15:10:11  rnovak
 * Added a Header.
 * 
 * Revision 1.2  86/12/29  15:03:25  rnovak
 * Added a log.
 * 
 */
#include <boolean.h>
#include <malloc.h>

char *
strcar(string,delimit)
char *	string ;
char *	delimit ;
{
	char *	return_val ;
	int	length1 ;
	int	length2 ;
	boolean	found = FALSE ;
	int	i ;
	int	j ;

	length1 = strlen(string) ;
	length2 = strlen(delimit) ;
	return_val = malloc(length1+1) ;
	for (i=0;i<length1;i++) {
		for (j=0;j<length2;j++) {
			if (string[i] == delimit[j]) {
				found = TRUE ;
			}
		}
		if (found == TRUE) break ;
		return_val[i] = string[i] ;
	}
	return_val[i] = EOS ;
	return return_val ;
}
