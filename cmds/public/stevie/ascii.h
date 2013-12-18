/*
 * Definitions of various common control characters
 *
 * $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/ascii.h,v 1.1 1993/08/06 15:17:14 nickc Exp $
 *
 * $Log: ascii.h,v $
 * Revision 1.1  1993/08/06  15:17:14  nickc
 * Initial revision
 *
 * Revision 1.1  88/03/20  21:03:24  tony
 * Initial revision
 * 
 *
 */

#define	NUL	'\0'
#define	BS	'\010'
#define	TAB	'\011'
#define	NL	'\012'
#define	CR	'\015'
#define	ESC	'\033'

#define	CTRL(x)	((x) & 0x1f)
