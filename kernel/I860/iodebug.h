/*
 *	==== i860 DEBUGGING MESSAGE LIBRARY ====
 *		  ==== HEADER ====
 *
 *  This library provides routines for outputting debugging messages
 *  via the link and the i860 server.
 *
 *	Ver	Date		By	Modifications
 *	1.0	1990-08-01	RPTB	Initial version.
 */

#ifndef _iodebug_h
#define _iodebug_h


/*  === WRITE FORMATTED OUTPUT ===
 *
 *  This function provides debugging output similar to a simplified
 *  printf function, but at a much lower level. The only supported
 *  formats are %d %x %s and %c, without modifiers.
 */

void _debug(const char *format, ...);


#endif

/* End of header */
