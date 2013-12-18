#ifndef lint
static char *RCSid = "$Header: min.c,v 1.1 87/08/21 16:33:49 rnovak Exp $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /u3/syseng/rnovak/src/lib/RCS/min.c,v $
 * $Revision: 1.1 $
 * $Date: 87/08/21 16:33:49 $
 * $State: Exp $
 * $Author: rnovak $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 * $Log:	min.c,v $
 * Revision 1.1  87/08/21  16:33:49  rnovak
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

/*
 *	$Header: min.c,v 1.1 87/08/21 16:33:49 rnovak Exp $
 *	$Log:	min.c,v $
 * Revision 1.1  87/08/21  16:33:49  rnovak
 * Initial revision
 * 
 * Revision 1.5  86/12/29  11:38:37  rnovak
 * Copied from ~/src/general/oa/month
 * 
 * Revision 1.5  86/12/29  11:38:37  rnovak
 * Final version of standard header.
 * 
 */
min(a,b) int a,b ; { return((a<b)?a:b) ; }
