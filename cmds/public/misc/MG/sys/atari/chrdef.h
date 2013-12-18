/* chrdef.h -- sample character attribute macros
 *
 * author :  Sandra Loosemore
 * date   :  26 Oct 1987
 *
 * This file works for the Atari ST 8-bit character set.  It probably
 *	won't work for other character encodings.
 *
 */


#define _W	0x01			/* Word.			*/
#define _U	0x02			/* Upper case letter.		*/
#define _L	0x04			/* Lower case letter.		*/
#define _C	0x08			/* Control.			*/
#define _P	0x10			/* end of sentence punctuation	*/

#define ISWORD(c)	((cinfo[(c)]&_W)!=0)
#define ISCTRL(c)	((cinfo[(c)]&_C)!=0)
#define ISUPPER(c)	((cinfo[(c)]&_U)!=0)
#define ISLOWER(c)	((cinfo[(c)]&_L)!=0)
#define ISEOSP(c)	((cinfo[(c)]&_P)!=0)
#define TOUPPER(c)	(cupper[(c)])
#define TOLOWER(c)	(clower[(c)])
#define ISDIGIT(c)	(((c) >= '0') && ((c) <= '9'))

extern char cupper[];
extern char clower[];
extern char cinfo[];


/*
 * generally useful thing for chars
 */
#define CCHR(x)		((x) ^ 0x40)	/* CCHR('?') == DEL */
#define CHARMASK(c)	((c)&0xff)
