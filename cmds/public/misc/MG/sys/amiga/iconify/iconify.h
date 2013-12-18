/*  :ts=8 bk=0
 *
 * iconify.h:	Should be used by all programs intending to use iconify.c.
 *
 * Copyright 1987 by Leo L. Schwab.
 * Permission is hereby granted for use in any and all programs, both
 * Public Domain and commercial in nature, provided this Copyright notice
 * is left intact.  Purveyors of programs, at their option, may wish observe
 * the following conditions (in the spirit of hackerdom):
 *	1: You send me a free, registered copy of the program that uses the
 *	   iconify feature,
 *	2: If you're feeling really nice, a mention in the program's
 *	   documentation of my name would be neat.
 *
 *			 		8712.10		(415) 456-3960
 */

#define	ICON_IMAGE	0
#define	ICON_BORDER	1
#define	ICON_FUNCTION	2

/*  Suggested icon size for a standard (640 x 200) WorkBench screen.  */
#define	ICONWIDTH	((UWORD) 50)
#define	ICONHEIGHT	((UWORD) 25)
