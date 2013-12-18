/*	CHAR.C:	Character handling functions for
		MicroEMACS 3.10
		(C)opyright 1988 by Daniel Lawrence

		ALL THE CODE HERE IS FOR VARIOUS FORMS OF ASCII AND
		WILL HAVE TO BE MODIFIED FOR EBCDIC
*/

#include	<stdio.h>
#include	"estruct.h"
#include	"etype.h"
#include	"edef.h"
#include	"elang.h"

#if	DIACRIT
/*	isletter()
		Is the character a letter?  We presume a letter must
	be either in the upper or lower case tables (even if it gets
	translated to itself).
*/

int PASCAL NEAR isletter(ch)

register unsigned int ch;

{
	return(isupper(ch) || islower(ch));
}

/*	islower()
		Is the character a lower case letter?  This looks
	in the lower to uppercase translation table.
*/

int PASCAL NEAR islower(ch)
register unsigned int	ch;
{
	return(lowcase[ch] != 0);
}

/*	isupper()
		Is the character a upper case letter?  This looks
	in the upper to lowercase translation table.
*/

int PASCAL NEAR isupper(ch)
register unsigned int	ch;
{
	return(upcase[ch] != 0);
}

/*	chcase()

		Change the case of the current character.
	First check lower and then upper.  If it is not a letter,
	it gets returned unchanged.
*/

unsigned int PASCAL NEAR chcase(ch)
register unsigned int	ch;
{
	/* translate lowercase */
	if (islower(ch))
		return(lowcase[ch]);

	/* translate uppercase */
	if (isupper(ch))
		return(upcase[ch]);

	/* let the rest pass */
	return(ch);
}

/* change *cp to an upper case character */

uppercase(cp)

char *cp;	/* ptr to character to uppercase */

{
	/* translate lowercase */
	if (islower(*cp))
		*cp = lowcase[*cp];
}

/* change *cp to an lower case character */

lowercase(cp)

char *cp;	/* ptr to character to lowercase */

{
	/* translate lowercase */
	if (isupper(*cp))
		*cp = upcase[*cp];
}

char PASCAL NEAR upperc(ch)	/* return the upper case equivalant of a character */

char ch;	/* character to get uppercase euivalant of */

{
	if (islower(ch))
		return(lowcase[ch]);
	else
		return(ch);
}

char PASCAL NEAR lowerc(ch)	/* return the lower case equivalant of a character */

char ch;	/* character to get lowercase equivalant of */

{
	if (isupper(ch))
		return(upcase[ch]);
	else
		return(ch);
}

PASCAL NEAR initchars()	/* initialize the character upper/lower case tables */

{
	register int index;	/* index into tables */

	/* all of both tables to zero */
	for (index = 0; index < HICHAR; index++) {
		lowcase[index] = 0;
		upcase[index] = 0;
	}

	/* lower to upper */
	for (index = 'a'; index <= 'z'; index++)
		lowcase[index] = index - DIFCASE;

	/* upper to lower */
	for (index = 'A'; index <= 'Z'; index++)
		upcase[index] = index + DIFCASE;

#if	MSDOS
	/* setup various extended IBM-PC characters */
	upcase[0x80]  = 0x87;	/* C with a cedilla */
	lowcase[0x81] = 0x9a;	/* U with an umlat */
	lowcase[0x82] = 0x90;	/* E with an acute accent */
	lowcase[0x83] = 0x83;	/* A with two dots */
	lowcase[0x84] = 0x8e;	/* A with an umlat */
	lowcase[0x85] = 0x85;	/* A with a grave accent */
	lowcase[0x86] = 0x8f;	/* A with a circle */
	lowcase[0x87] = 0x80;	/* C with a cedilla */
	lowcase[0x88] = 0x88;	/* E with a ^ */
	lowcase[0x89] = 0x89;	/* E with two dots */
	lowcase[0x8a] = 0x8a;	/* E with a grave accent */
	lowcase[0x8b] = 0x8b;	/* I with two dots */
	lowcase[0x8c] = 0x8c;	/* I with a ^ */
	lowcase[0x8d] = 0x8d;	/* I with a grave accent */
	upcase[0x8e]  = 0x84;	/* A with an umlat */
	upcase[0x8f]  = 0x86;	/* A with a circle */
	upcase[0x90]  = 0x82;	/* E with an acute accent */
	lowcase[0x91] = 0x92;	/* AE combination */
	upcase[0x92]  = 0x91;	/* AE combination */
	lowcase[0x93] = 0x93;	/* O with a ^ */
	lowcase[0x94] = 0x99;	/* O with an umlat */
	lowcase[0x95] = 0x95;	/* O with an acute accent */
	lowcase[0x96] = 0x96;	/* U with a ^ */
	lowcase[0x97] = 0x97;	/* U with an grave accent */
	lowcase[0x98] = 0x98;	/* Y with two dots */
	upcase[0x99]  = 0x94;	/* O with an umlat */
	upcase[0x9a]  = 0x81;	/* U with an umlat */
	lowcase[0xa0] = 0xa0;	/* A with an acute accent */
	lowcase[0xa1] = 0xa1;	/* I with an acute accent */
	lowcase[0xa2] = 0xa2;	/* O with an acute accent */
	lowcase[0xa3] = 0xa3;	/* U with an acute accent */
	lowcase[0xa4] = 0xa5;	/* N with a ...... */
	upcase[0xa5]  = 0xa4;	/* N with a ...... */
	lowcase[0xa6] = 0xa6;	/* A underlined */
	lowcase[0xa7] = 0xa7;	/* O underlined */
#endif
}

/*	Set a character in the lowercase map */

int PASCAL NEAR setlower(ch, val)

char *ch;	/* ptr to character to set */
char *val;	/* value to set it to */

{
	return(lowcase[*ch & 255] = *val & 255);
}

/*	Set a character in the uppercase map */

int PASCAL NEAR setupper(ch, val)

char *ch;	/* ptr to character to set */
char *val;	/* value to set it to */

{
	return(upcase[*ch & 255] = *val & 255);
}
#else
/* change *cp to an upper case character */

uppercase(cp)

char *cp;	/* ptr to character to uppercase */

{
	/* translate lowercase */
	if (islower(*cp))
		*cp -= DIFCASE;
}

/* change *cp to an lower case character */

lowercase(cp)

char *cp;	/* ptr to character to lowercase */

{
	/* translate lowercase */
	if (isupper(*cp))
		*cp += DIFCASE;
}

char PASCAL NEAR upperc(ch)	/* return the upper case equivalant of a character */

char ch;	/* character to get uppercase euivalant of */

{
	if (islower(ch))
		return(ch - DIFCASE);
	else
		return(ch);
}

char PASCAL NEAR lowerc(ch)	/* return the lower case equivalant of a character */

char ch;	/* character to get lowercase equivalant of */

{
	if (isupper(ch))
		return(ch + DIFCASE);
	else
		return(ch);
}

PASCAL NEAR initchars()	/* initialize the character upper/lower case tables */

{
	/* there is nothing we need to do here! */
}

/*	Set a character in the lowercase map */

int PASCAL NEAR setlower(ch, val)

char *ch;	/* ptr to character to set */
char *val;	/* value to set it to */

{
	return(*val & 255);
}

/*	Set a character in the uppercase map */

int PASCAL NEAR setupper(ch, val)

char *ch;	/* ptr to character to set */
char *val;	/* value to set it to */

{
	return(*val & 255);
}
#endif
