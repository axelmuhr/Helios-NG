/*
 * Varargs, for use on Atari ST (works with MWC, probably others).
 *	Came from AmigaDOS version, which was borrowed from 4BSD.
 */

typedef char		*va_list;
#define va_dcl		int va_alist;
#define va_start(pv)	pv = (char *) &va_alist
#define va_end(pv)	/* Naught to do... */
#define va_arg(pv, t)	((t *) (pv += sizeof(t)))[-1]
