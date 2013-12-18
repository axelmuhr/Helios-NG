/****************************************************************/
/*                          Ariel Corp.                         */
/*                        433 River Road                        */
/*                Highland Park, NJ 08904, U.S.A.               */
/*                     Tel:  (908) 249-2900                     */
/*                     Fax:  (908) 249-2123                     */
/*                     BBS:  (908) 249-2124                     */
/*                  E-Mail:  ariel@ariel.com                    */
/*                                                              */
/*                 Copyright (C) 1993 Ariel Corp.               */
/****************************************************************/


/* $Id: coff.h,v 1.1 1994/06/29 13:46:19 tony Exp $ */

/* coff.h
 *
 * These definitions are needed by user programs for the COFF functions.
 */

#ifndef COFF_H
#define COFF_H 1

#include "c40types.h"      /* for u_long */

#ifndef TRUE
#	define	TRUE	1
#endif
#ifndef FALSE
#	define	FALSE	0
#endif

extern char cofferr[];

/*
 * symbol storage classes
 */
#define	C_NULL	0	/* no class */
#define	C_AUTO	1	/* automatic var */
#define	C_EXT	2	/* external symbol */
#define	C_STAT	3	/* static */
#define	C_REG	4	/* register variable */
#define	C_EXTDEF 5	/* external definition */
#define	C_LABEL	6	/* label */
#define	C_ULABEL 7	/* undefined label */
#define	C_MOS	8	/* structure member */
#define	C_ARG	9	/* function argument */	
#define	C_STRTAG 10	/* structure tag */
#define	C_MOU	11	/* member of union */
#define	C_UNTAG	12	/* union tag */
#define	C_TPDEF	13	/* type definition */
#define	C_USTATIC 14	/* undefined static */
#define	C_ENTAG	15	/* enumeration tag */
#define	C_MOE	16	/* member of enumeration */
#define	C_REGPARM 17	/* register parameter */
#define	C_FIELD	18	/* bit field */
#define	C_UEXT	19	/* tentative definition */
#define	C_STATLAB 20	/* static .label symbol */
#define	C_EXTLAB 21	/* external .label symbol */
#define	C_BLOCK	100	/* beginning or end of a block */
#define	C_FCN	101	/* beginning or end of a function */
#define	C_EOS	102	/* end of structure */
#define	C_FILE	103	/* filename */
#define	C_LINE	104	/* used only by utility programs */

/*
 * symbol types
 */
#define	T_NULL	0x80
#define	T_VOID	0
#define	T_SCHAR	1
#define	T_CHAR	2
#define	T_SHORT	3
#define	T_INT	4
#define	T_LONG	5
#define	T_FLOAT	6
#define	T_DOUBLE 7
#define	T_STRUCT 8
#define	T_UNION	9
#define	T_ENUM	10
#define	L_DOUBLE 11
#define	T_UCHAR	12
#define	T_USHORT 13
#define	T_UINT	14
#define	T_ULONG	15
#define	T_UNDEF	0xff

/*
 * user symbol table structure
 */
struct symtab {
	char *sname;
	union {
		u_long	l;
		float	f;
	} val;
	short type;
	char class;
};


#endif /* COFF_H */
