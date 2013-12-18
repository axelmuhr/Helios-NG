#ifndef lint
static char *RCSid = "$Header: strcdr.c,v 1.1 87/08/21 16:34:19 rnovak Exp $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /u3/syseng/rnovak/src/lib/RCS/strcdr.c,v $
 * $Revision: 1.1 $
 * $Date: 87/08/21 16:34:19 $
 * $State: Exp $
 * $Author: rnovak $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 * $Log:	strcdr.c,v $
 * Revision 1.1  87/08/21  16:34:19  rnovak
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

/*
 *	$Header: strcdr.c,v 1.1 87/08/21 16:34:19 rnovak Exp $
 *	$Log:	strcdr.c,v $
 * Revision 1.1  87/08/21  16:34:19  rnovak
 * Initial revision
 * 
 * Revision 1.2  86/12/29  16:10:22  rnovak
 * Handle the includes for an include directory.
 * 
 * Revision 1.1  86/12/29  15:24:07  rnovak
 * Initial revision
 * 
 * Revision 1.3  86/12/29  15:10:22  rnovak
 * Added a Header.
 * 
 * Revision 1.2  86/12/29  15:03:32  rnovak
 * Added a log.
 * 
 */
#include <boolean.h>
#include <malloc.h>
char *
strcdr(string,delimit)
char *	string ;
char *	delimit ;
{
	char *	return_val ;
	int	length1 ;
	int	length2 ;
	boolean	found = FALSE ;
	int	i ;
	int	j ;
	int	k = 0 ;

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
	}
	return_val[k] = EOS ;
	if (found == FALSE) {
		return return_val ;
	}
	for (;(i<length1)&&(found==TRUE);i++) {
		for (j=0;j<length2;j++) {
			if (string[i] != delimit[j]) {
				found = FALSE ;
			}
		}
	}
	strcpy(return_val,&string[i]) ;
	return return_val ;
}
