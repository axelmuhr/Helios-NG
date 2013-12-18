#ifndef lint
static char *RCSid = "$Header: max.c,v 1.1 87/08/21 16:33:40 rnovak Exp $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /u3/syseng/rnovak/src/lib/RCS/max.c,v $
 * $Revision: 1.1 $
 * $Date: 87/08/21 16:33:40 $
 * $State: Exp $
 * $Author: rnovak $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 * $Log:	max.c,v $
 * Revision 1.1  87/08/21  16:33:40  rnovak
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

/*
 *	$Header: max.c,v 1.1 87/08/21 16:33:40 rnovak Exp $
 *	$Log:	max.c,v $
 * Revision 1.1  87/08/21  16:33:40  rnovak
 * Initial revision
 * 
 * Revision 1.1  86/12/29  15:23:14  rnovak
 * Initial revision
 * 
 * Revision 1.3  86/12/29  15:09:38  rnovak
 * Added a Header.
 * 
 * Revision 1.2  86/12/29  15:01:19  rnovak
 * Clean up and add a log.
 * 
 */
int max(a,b) int a,b; { return ((a>b)?a:b) ; }
