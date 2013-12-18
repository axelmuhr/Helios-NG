/*
 * Varargs, for use on AmigaDOS with the Lattice C compiler,
 *	or (maybe?) the Manx compiler with 32-bit ints.
 *	Blatantly lifted from 4.2BSD.
 */

typedef char		*va_list;
#define va_dcl		int va_alist;
#define va_start(pv)	pv = (char *) &va_alist
#define va_end(pv)	/* Naught to do... */
#define va_arg(pv, t)	((t *) (pv += sizeof(t)))[-1]
