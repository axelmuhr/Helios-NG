#ifndef lint
static char *RCSid = "$Header: mstrcpy.c,v 1.1 87/08/21 16:33:58 rnovak Exp $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /u3/syseng/rnovak/src/lib/RCS/mstrcpy.c,v $
 * $Revision: 1.1 $
 * $Date: 87/08/21 16:33:58 $
 * $State: Exp $
 * $Author: rnovak $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 * $Log:	mstrcpy.c,v $
 * Revision 1.1  87/08/21  16:33:58  rnovak
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

/*
 *	$Header: mstrcpy.c,v 1.1 87/08/21 16:33:58 rnovak Exp $
 *	$Log:	mstrcpy.c,v $
 * Revision 1.1  87/08/21  16:33:58  rnovak
 * Initial revision
 * 
 * Revision 1.2  86/12/29  16:09:11  rnovak
 * Handle the includes for an include directory
 * 
 * Revision 1.1  86/12/29  15:23:37  rnovak
 * Initial revision
 * 
 * Revision 1.3  86/12/29  15:09:49  rnovak
 * Added a Header.
 * 
 * Revision 1.2  86/12/29  15:02:55  rnovak
 * Added a log.
 * 
 */
#include <malloc.h>
char *
mstrcpy(str)
char * str ;
{
	char *	temp ;

	temp = malloc(strlen(str)+1) ;
	strcpy(temp,str) ;
	return(temp) ;
}
