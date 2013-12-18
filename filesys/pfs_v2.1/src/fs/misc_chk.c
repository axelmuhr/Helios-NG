/* 16.08.90 - Some basic cuts to build the "integrated" version
 *
 *
 */
 
                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                  (c) 1988-91 by parsytec GmbH, Aachen                   |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                          Parsytec File System                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  misc_chk.c						             |
   |                                                                         |
   |    Various assistant procedures for the file system checker             |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O.Imbusch - 30 April 1991 - Error handling centralized           |
   |    1 - H.J.Ermen - 21 March 1989 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define	 DEBUG	    0
#define	 GEPDEBUG   0
#define	 FLDEBUG    0
#define  IN_NUCLEUS 1

#include "error.h"

#include "check.h"
#include "fserr.h"
#include <stdarg.h>


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
 VD *vol = &volume[bp->b_dev]; 	

 					/* Keep track on modified blocks */
 if ( bit_maps_allocated )		/* ... only, if we have a valid	 */
 {					/* reference bitmap.		 */
 					/* Set the specific bit	!	 */
DEBdebug ("	test_bwrite :	Note block no %d in reference-map as modified", bp->b_bnr );

 	bit_maps[bp->b_bnr/vol->incore_fs.fs_cgsize][bp->b_bnr%vol->incore_fs.fs_cgsize] |=
 		 MASK_MODIFIED;
 }
 					/* The 'look only' mode causes	 */
 if ( no_corrections || volume[cdev].writeprotected)	/* us to relase blocks.		 */
 {
 	Report (FSErr [Update], S_INFO, bp->b_bnr);
 	brelse ( bp->b_tbp, TAIL );
 }					/* Otherwise we always write 	*/
 else					/* them directly to disk.	*/
 	if (!bwrite ( bp->b_tbp )) {
 		Error (FSErr [BlkWriteFailed], S_FATAL,bp->b_tbp->p_fbp->b_bnr);
 		longjmp (term_jmp, WRITE_ERR);
 	}
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
