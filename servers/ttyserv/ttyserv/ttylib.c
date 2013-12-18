/*************************************************************************
**									**
**	       T E R M I N A L   W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** ttylib.c								**
**									**
**	- Server library (functions which not in the Util Library)	**
**	  Copied from the clib sources.					**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Source    :  most of the original winlib.h   (date 02/01/90)		**
*************************************************************************/

/* sort.c: ANSI draft (X3J11 Oct 86) library, section 4.10 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.02 */

#define __in_sort  1		/* flag that we are in this module */

#include <stddef.h>
#include <stdlib.h>


/* Qsort is implemented using an explicit stack rather than C recursion  */
/* See Sedgewick (Algorithms, Addison Wesley, 1983) for discussion.      */


#define STACKSIZE 32
#define SUBDIVISION_LIMIT 10

#define exchange(p1, p2, size)                      \
{                                                   \
    int xsize = size;                               \
    if (((xsize | (int) p1) & 0x3)==0)              \
    {   do                                          \
        {   int temp = *(int *)p1;                  \
            *(int *)p1 = *(int *)p2;                \
            *(int *)p2 = temp;                      \
            p1 += 4;                                \
            p2 += 4;                                \
            xsize -= 4;;                            \
        } while (xsize != 0);                       \
    }                                               \
    else                                            \
    {   do                                          \
        {   char temp = *p1;                        \
            *p1++ = *p2;                            \
            *p2++ = temp;                           \
            xsize--;                                \
        } while (xsize != 0);                       \
    }                                               \
    p1 -= size;                                     \
    p2 -= size;                                     \
}

#define stack(p, n)                                 \
    (basestack[stackpointer] = (void *) (p),        \
     sizestack[stackpointer++] = (n))



static void partition_sort(void *base, size_t nmemb, size_t size,
                           int (*compar)(const void *, const void *))
{
    int stackpointer = 1;
    void *basestack[STACKSIZE];
    int sizestack[STACKSIZE];
/* The explicit stack that is used here is large enough for any array    */
/* that could exist on this computer - since I stack sub-intervals in a  */
/* careful order I can guarantee that the depth of stack needed is       */
/* bounded by log2(nmemb), so I can certainly handle arrays of up to     */
/* size 2**STACKSIZE, which is bigger than my address space.             */
/* I require that SUBDIVISION_LIMIT >= 2 so that all segments to be      */
/* partitioned have at least 3 items in them. This means that the first, */
/* middle and last items in a segment are distinct (although they may,   */
/* of course, have the same value).                                      */
    basestack[0] = base;
    sizestack[0] = nmemb;
    while (stackpointer!=0)
    {   char *b = (char *) basestack[--stackpointer];
        int n = sizestack[stackpointer];
/* The for loop here marks where we re-enter the code having refined an  */
/* interval.                                                             */
        for (;;)
        {   int halfn = (n >> 1) * size;  /* index of middle in the list */
            char *bmid = b + halfn;
            char *btop;
            if ((n & 1) == 0) halfn -= size;
            btop = bmid + halfn;
/* Sort first, middle and last items in the segment into order. Put the  */
/* smallest and largest at the start and end of the segment (where they  */
/* act as sentinel values in a conveniant way) and use the median of 3   */
/* as my pivot. This happens to ensure that sorted and inverse sorted    */
/* input gets pivoted neatly and costs n*log n operations, even though   */
/* there are STILL worst cases that take about n*n operations.           */
            if ((*compar)(b, bmid) > 0) exchange(b, bmid, size);
            if ((*compar)(bmid, btop) > 0)
            {   if ((*compar)(b, btop) > 0) exchange(b, btop, size);
                exchange(bmid, btop, size);
            }
/* Now I have the first and last elements in place & a useful pivot.     */
            {   char *l;
                char *r = btop - size;
                char *pivot;
                int s1, s2;
                exchange(bmid, r, size);
                l = b;
                pivot = r;
/* Sweep inwards partitioning elements on the basis of the pivot         */
                for (;;)
                {   do l += size; while ((*compar)(l, pivot) < 0);
                    do r -= size; while ((*compar)(r, pivot) > 0);
                    if (r <= l) break;
                    exchange(l, r, size);
                }
/* Move the pivot value to the middle of the array where it wants to be. */
                exchange(l, pivot, size);
/* s1 and s2 get the sizes of the sublists that I have partitioned my    */
/* data into. Note that s1 + s2 = n - 1 since the pivot element is now   */
/* in its correct place.                                                 */
                s1 = ((l - b)/ size) - 1;
                s2 = n - s1 - 1;
/* I am not going to try partitioning things that are smaller than some  */
/* fairly arbitrary small size (SUBDIVISION_LIMIT), and (only) if I will */
/* have to subdivide BOTH remaining intervals I push the larger of them  */
/* onto a stack for later consideration.                                 */
                if (s1 > s2)
/* Here the segment (b, s1) is the larger....                            */
                {   if (s2 > SUBDIVISION_LIMIT)
/* If the smaller segment needs subdividing then a fortiori the larger   */
/* one does, and needs stacking.                                         */
                    {   stack(b, s1);
                        b = l;
                        n = s2;
                    }
/* If just the larger segment needs dividing more I can loop.            */
                    else if (s1 > SUBDIVISION_LIMIT) n = s1;
/* If both segments are now small I break out of the loop.               */
                    else break;
                }
                else
/* Similar considerations apply if (l, s2) is the larger segment.        */
                {   if (s1 > SUBDIVISION_LIMIT)
                    {   stack(l, s2);
                        n = s1;
                    }
                    else if (s2 > SUBDIVISION_LIMIT)
                    {   b = l;
                        n = s2;
                    }
                    else break;
                }
            }
        }
    }
}

void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *))
{
/* Sort an array (base) with nmemb items each of size bytes.             */
/* This uses a quicksort method, but one that is arranged to complete in */
/* about n*log(n) steps for sorted and inverse sorted inputs.            */
    char *b, *endp;
    if (size <= 0) return;
    if (nmemb > SUBDIVISION_LIMIT)
        partition_sort(base, nmemb, size, compar);
/* Now I do an insertion sort on the array that is left over. This makes */
/* sense because the quicksort activity above has arranged that the      */
/* array is nearly sorted - at most segments of size SUBDIVISION_LIMIT   */
/* contain locally unsorted segments. In this case insertion sort goes   */
/* as fast as anything else. Doing things this way avoids the need for   */
/* a certain amount of partitioning/recursing overhead in the main body  */
/* of the quicksort procedure.                                           */
/* Note: some sorts of bugs in the above quicksort could be compensated  */
/* for here, leaving this entire procedure as just an insertion sort.    */
/* This would yield correct results but would be somewhat slow.          */
    b = (char *) base;
    endp = b + (nmemb-1) * size;
    while (b < endp)
    {   char *b1 = b;
        b += size;
/* Find out where to insert this item.                                   */
        while (b1>=(char *)base && (*compar)(b1, b)>0) b1 -= size;
        b1 += size;
/* The fact that I do not know how large the objects that are being      */
/* sorted are is horrible - I do an exchange here copying things one     */
/* word at a time or one byte at a time depending on the value of size.  */
        if (((size | (int) b) & 0x3)==0)
        {   int j;
            for (j=0; j<size; j+=4)
            {   char *bx = b + j;
/* Even when moving word by word I use (char *) pointers so as to avoid  */
/* muddles about pointer arithmetic. I cast to (int *) pointers when I   */
/* am about to dereference something.                                    */
                int save = *(int *)bx;
                char *by = bx - size;
                while (by>=b1)
                {   *(int *)(by + size) = *(int *)by;
                    by -= size;
                }
                *(int *)(by + size) = save;
            }
        }
        else
/* If size is not a multiple of 4 I behave with great caution and copy   */
/* byte by byte. I could, of course, copy by words up to the last 1, 2   */
/* or 3 bytes - I count that as too much effort for now.                 */
        {   int j;
            for (j=0; j<size; j++)
            {   char *bx = b + j;
                char save = *bx;
                char *by = bx - size;
                while (by>=b1)
                {   *(by + size) = *by;
                    by -= size;
                }
                *(by + size) = save;
            }
        }
    }
}

/* string.c: ANSI draft (X3J11 Oct 86) library code, section 4.11 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.03 */

#define __in_string  1		/* flag that we are in this module */

#include <stddef.h>

void *
memmove (void *a, const void *b, size_t n)
/* copy memory taking care of overlap */
/* Relies on sizeof (int) = sizeof (void *) and byte addressing.
   Also that memory does not wrap round for direction test. */
{
    char	*ca,*cb;
      
    if (a < (void *) b)
        for (ca = (char *) a, cb = (char *) b; n-- > 0;) 
            *ca++ = *cb++;
    else 
    	for (ca = n + (char *) a, cb = n + (char *) b; n-- > 0;) 
    	    *--ca = *--cb;
    return a;
}

int 
memcmp (const void *a, const void *b, size_t n)
{
    unsigned char *ca = a, *cb = b;
    while (n-- > 0)
        if (*ca++ != *cb++) return *--ca - *--cb;
    return 0;
}

char *
strchr (const char *s, int ch)
                                        /* find first instance of ch in s */
{   
    for (;; s++)
    {   
    	if (*s == (char) ch) 
    	    return (char *) s;
        if (*s == 0) 
            return (char *) NULL;
    }
}

size_t 
strspn (const char *s, const char *p)
                                        /* find first char in s not in p */
{   
    const char *ss, *pp;
    char ch;
    for (ss = s;; ss++)
    {   
    	if ((ch = *ss) == 0) 
    	    return ss - s;
        for (pp = p;; pp++) 
        {   
            if (*pp == 0) 
            	return ss - s;
            if (*pp == ch) 
            	break;
        }
    }
}

char *
strpbrk(const char *s, const char *p)
                                        /*  ditto, except ptr/NULL result */
{   const char *ss, *pp;
    char ch;
    for (ss = s;; ss++)
    {   
    	if ((ch = *ss) == 0) 
    	    return (char *) NULL;
        for (pp = p; *pp != 0; pp++) 
            if (*pp == ch) 
            	return (char *) ss;
    }
}

#include "../debug/debug.h"

extern int		opterr;		/* enable error messages	*/
extern int		optind;		/* index into argv vector	*/
extern int		optopt;		/* char checked for validity	*/
extern char		*optarg;	/* option's associated argument */

#define	ERRMSG(s)	if ( opterr ) DoDebug ( "%s%s%c", *argv, s, optopt );

int
getopt ( int argc, char **argv, char *optmask )
{
    static char	*aptr	= "";		/* argument scanning pointer	*/
    char	*optr;			/* option list pointer		*/

    if ( ! *aptr )			/* end of argument reached:	*/
    {
					/* update scanning pointer	*/
	if ( optind >= argc || *( aptr = argv [optind] ) != '-' )
	    return -1;

	if ( aptr[1] && *++aptr == '-' )	/* found "--"		*/
	{
	    optind++;
	    return -1;
	}
    }
    optopt = ( int ) *aptr++;		/* set option letter		*/
    optr = strchr ( optmask, optopt );
					/* option letter ok ?		*/
    if ( optopt == ( int ) ':' || optr == NULL )
    {
	if (! *aptr )
	    optind++;
	ERRMSG ( " : unknown option -- " );
	return '?';
    }

    if ( *++optr == ':' )		/* option needs an argument	*/
    {
	if ( *aptr )			/* argument after the letter	*/
	{
	    optarg = aptr;		/* using the rest of the arg as	*/
	    optind++;			/* value -> increment optind	*/
	}
	else if ( argc <= ++optind )	/* check for existence		*/
	{
	    aptr = "";
	    ERRMSG ( " : option needs an argument -- " );
	    optarg = NULL;
	    return '?';
	}
	else				/* use next argv entry as value	*/
	    optarg = argv [optind++];	/* skip over it			*/
	aptr = "";
    }
    else				/* no argument required		*/
    {
	optarg = NULL;
	if ( ! *aptr )
	    optind++;
    }
    return optopt;
}


/*************************************************************************
 * CALCULATE A NUMERIC DIGITS VALUE
 *
 * - If the character is not a valid digit
 *   or the value exceeds the radix, -1 is returned.
 *
 * Return	:	digit value
 *
 ************************************************************************/

static int
ch_val (char ch, int radix)
{
    int val = -1;

    if (ch >= '0' && ch <= '9') val = ch - '0';
    elif (ch >= 'a' && ch <= 'f') val = ch - 'a' + 10;
    elif (ch >= 'A' && ch <= 'F') val = ch - 'A' + 10;

    return (val < radix ? val : -1);
}


/*************************************************************************
 * CONVERT A STRING TO AN INTEGRAL VALUE
 *
 * - ascint expects the line to consist of the following:
 *   1. Leading white space (optional).
 *   2. A plus or minus sign (optional).
 *   3. An octal '0' or hexadecimal '0x' or '0X' prefix (optional).
 *   4. A sequence of digits within the range of the appropiate base.
 *   5. One or more unrecognised characters
 *	(including the terminating null char).
 *   If the result would overflow the conversion stops.
 *
 * Parameter  :	nptr	= string to convert
 * Return     :		converted value
 *
 ************************************************************************/

int
getint ( char *nptr )
{
    bool	ok	= FALSE;
    int		nflag	= 0;
    int		base	= 0;
    int		dlow	= 0;
    int		dhigh	= 0;
    int		dvalue;
    int		digit;
    char	c;
					/* skip white space		*/
    while ((c = *nptr++) != 0 && (c == ' ' || c == '\t')) ;

    if (c == '-')			/* check for sign		*/
    {
	nflag++;			/* negative: set flag, skip	*/
	c = *nptr++;
    }
    elif (c == '+')
	c = *nptr++;			/* positive: skip		*/

    if (c == '0' && *nptr != '\0')	/* get base value		*/
    {
	c = *nptr++;			/* leading '0': octal or hex	*/
	if (c == 'x' || c == 'X')
	{
	    base = 16;			/* is hex...			*/
	    c = *nptr++;
	}
	else
	    base = 8;			/* is octal...			*/
    }
    else
	base = 10;			/* default is decimal.		*/

    while ((digit = ch_val (c, base)) >= 0)	/* now collect value.	*/
    {
	dlow = base * dlow + digit;		/* low half		*/
	dhigh = base * dhigh + (dlow >> 16);	/* high half		*/
	dlow &= 0xffff;
	if (dhigh >= 0x8000)			/* check for overflow	*/
	{
	    ok = FALSE;
	    break;
	}
	ok = TRUE;
	dvalue = (dhigh << 16) | dlow;		/* get final value	*/
	c = *nptr++;
    }
    if (ok)
	return (nflag ? -dvalue : dvalue);	/* return signed value.	*/
    else
	return -1;
}

/*--- end of ttylib.c ---*/
