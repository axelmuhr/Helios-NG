/* 16.08.90 - Some basic cuts to build the "integrated" version
 *
 *
 */
 
/*************************************************************************
**                                                                      **
**           H E L I O S   F I L E S Y S T E M   C H E C K E R          **
**           -------------------------------------------------          **
**                                                                      **
**                  Copyright (C) 1989 Parsytec GmbH                    **
**                         All Rights Reserved.                         **
**                                                                      **
** misc_chk.c								**
**                                                                      **
**	Various assistant procedures for the file system checker	**
**                                                                      **
**************************************************************************
** HISTORY   :                                                          **
**------------       							**
** Author    :  21/03/89 : H.J.Ermen 					**
*************************************************************************/

#include "check.h"
#include <stdarg.h>

#define	 DEBUG	FALSE

/*-----------------------------------------------------------------------*/

/*************************************************************************
 * CHECK THE "LOOK-ONLY" FLAG no_corrections (GLOBAL) AND DECIDE WHETHER TO
 * WRITE OR RELEASE A BLOCK
 *
 * - This procedure is used to ease the handling of the two general operating
 *   modes 
 *	"check and correct" : no_corrections = FALSE    and
 *      "only check"        : no_corrections = TRUE  
 *
 * Parameter  : bp = The pointer to the buffer to be written
 * Return     : - nothing -
 *
 *************************************************************************/
void test_bwrite ( struct buf *bp )
{
 					/* Keep track on modified blocks */
 if ( bit_maps_allocated )		/* ... only, if we have a valid	 */
 {					/* reference bitmap.		 */
 					/* Set the specific bit	!	 */
#if DEBUG
IOdebug ("	test_bwrite :	Note block no %d in reference-map as modified",
	 bp->b_bnr );
#endif
 	bit_maps[bp->b_bnr/incore_fs.fs_cgsize][bp->b_bnr%incore_fs.fs_cgsize] |=
 		 MASK_MODIFIED;
 }
 					/* The 'look only' mode causes	 */
 if ( no_corrections )			/* us to relase blocks.		 */
 {
 	IOdebug ("Block no. %d has to be updated !!", bp->b_bnr);
 	brelse ( bp->b_tbp, TAIL );
 }					/* Otherwise we always write 	*/
 else					/* them directly to disk.	*/
 	bwrite ( bp->b_tbp );
}

/*-------------------------------------------------------------------------*/


static char*
writenum (char *dest, unsigned int value, int base, int width)
{
	static char *digits = "0123456789abcdef";
	
	if ( width > 0 || value != 0 ) {
		dest = writenum (dest, value / base, base, width-1);
		*dest++ = digits [value%base];	
	}	
	return (dest);
}

void 
my_itoa ( char *buffer, word value )
{
	writenum (buffer, (unsigned)value,10,3);
}

/*-------------------------------------------------------------------------*/

 
int my_memcmp(const void *a, const void *b, size_t n)
{   const unsigned char *ac = a, *bc = b;
#ifdef _copywords
    if ((((int)ac | (int)bc) & 3) == 0)
    {   while (n >= 4 && *(int *)ac == *(int *)bc)
            ac += 4, bc += 4, n -= 4;
    }
#endif
    while (n-- > 0)
    {   unsigned char c1,c2;   /* unsigned cmp seems more intuitive */
        if ((c1 = *ac++) != (c2 = *bc++)) return c1 - c2;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

int
my_strcmp (char *s1, char *s2)

/*
 *  Return : TRUE, if both strings are equal, FALSE else
 */

{
	while ( !(*s1) && !(*s2) ) {
		if ( *s1 != *s2 )
			return (FALSE);
		s1++;
		s2++;	
	} 
	if ( *s1 != *s2 )
		return (FALSE);
	return (TRUE);
}


/*-------------------------------------------------------------------------*/

/* end of misc_chk.c */


