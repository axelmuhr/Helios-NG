/*
 * Name:	MicroEMACS
 *		Atari 520ST header file.
 * Version:	30
 * Last edit:	22-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#define	GOSLING	1			/* Use fancy redisplay.		*/
 
#define	NROW	50			/* The "50" is big enough to	*/
#define	NCOL	80			/* deal with the "hi50" screen.	*/
 
/*
 * Special keys.
 */

#define KFIRST	256
#define KLAST   284


/* These i/o functions are NOP's or direct equivalents of BIOS calls.
 *	Make them #define's so we don't have to go through a useless
 *	level of indirection.
 */

#define ttinit()
#define tttidy()
#define ttwindow(top,bot) (top, bot)
#define ttresize()
#define ttnowindow()
#define ttputc(c) Bconout(2, c)		/* Primitive output function	*/
#define ttflush()	                /* A NOP			*/
#define typeahead() ((int)Bconstat(2))	/* Check if there is input	*/
